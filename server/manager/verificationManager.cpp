#include "verificationManager.h"

#include <functional>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "user.h"
#include "groupid.hpp"
#include "userid.hpp"
#include "manager.h"

// manager
extern qls::Manager serverManager;

namespace qls {

struct FriendVerification
{
    UserID  applicator;
    UserID  controller;
};

struct GroupVerification
{
    UserID  applicator;
    GroupID controller;
};

} // namespace qls

namespace std {

template<>
struct hash<qls::FriendVerification>{
public:
    std::size_t operator()(const qls::FriendVerification &f) const 
    {
        hash<long long> h{};
        return h(f.applicator.getOriginValue()) ^
            h(f.controller.getOriginValue());
    }
};

template<>
struct equal_to<qls::FriendVerification>{
public:
    bool operator()(const qls::FriendVerification &f1,
        const qls::FriendVerification &f2) const
    {
        return f1.applicator == f2.applicator &&
            f1.controller == f2.controller;
    }
};

template<>
struct hash<qls::GroupVerification>{
public:
    std::size_t operator()(const qls::GroupVerification &g) const 
    {
        hash<long long> h{};
        return h(g.applicator.getOriginValue()) ^
            h(g.controller.getOriginValue());
    }
};

template<>
struct equal_to<qls::GroupVerification>{
public:
    bool operator()(const qls::GroupVerification &g1,
        const qls::GroupVerification &g2) const
    {
        return g1.applicator == g2.applicator &&
            g1.controller == g2.controller;
    }
};

} // namespce std

namespace qls {

struct VerificationManager::VerificationManagerImpl
{
    /**
     * @brief Map of friend room verification requests.
     */
    std::unordered_map<FriendVerification, bool>
                        m_friendRoomVerification_map;
    std::shared_mutex   m_friendRoomVerification_map_mutex;

    /**
     * @brief Map of group room verification requests.
     */
    std::unordered_map<GroupVerification, bool>
                        m_groupVerification_map;
    std::shared_mutex   m_groupVerification_map_mutex;
};

VerificationManager::VerificationManager():
    m_impl(std::make_unique<VerificationManagerImpl>())
{
}

VerificationManager::~VerificationManager() noexcept = default;

void VerificationManager::init()
{
    // sql init
}

void VerificationManager::applyFriendRoomVerification(UserID sender, UserID receiver)
{
    if (sender == receiver)
        throw std::system_error(qls_errc::invalid_verification);
    if (!serverManager.hasUser(sender))
        throw std::system_error(
            qls_errc::user_not_existed,
            "the id of sender is invalid");
    if (!serverManager.hasUser(receiver))
        throw std::system_error(
            qls_errc::user_not_existed,
            "the id of receiver is invalid");

    // check if they are friends
    if (!serverManager.hasPrivateRoom(sender, receiver))
        throw std::system_error(qls_errc::private_room_existed);

    {
        std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

        if (m_impl->m_friendRoomVerification_map.find(
            { sender, receiver })
            != m_impl->m_friendRoomVerification_map.cend())
            throw std::system_error(qls_errc::verification_existed);

        m_impl->m_friendRoomVerification_map.emplace(
            FriendVerification{ sender, receiver },
            false);
    }
    
    // sender
    {
        qls::Verification::UserVerification sender_uv;

        sender_uv.user_id = receiver;
        sender_uv.verification_type =
            qls::Verification::VerificationType::Sent;

        auto ptr = serverManager.getUser(sender);
        ptr->addFriendVerification(receiver, std::move(sender_uv));
    }

    // receiver
    {
        qls::Verification::UserVerification receiver_uv;

        receiver_uv.user_id = sender;
        receiver_uv.verification_type =
            qls::Verification::VerificationType::Received;

        auto ptr = serverManager.getUser(receiver);
        ptr->addFriendVerification(sender, std::move(receiver_uv));
    }
}

bool VerificationManager::hasFriendRoomVerification(UserID sender, UserID receiver) const
{
    if (sender == receiver)
        return false;

    std::shared_lock lock(m_impl->m_friendRoomVerification_map_mutex);
    return m_impl->m_friendRoomVerification_map.find(
        { sender, receiver }) !=
        m_impl->m_friendRoomVerification_map.cend();
}

void VerificationManager::acceptFriendVerification(UserID sender, UserID receiver)
{
    if (sender == receiver)
        throw std::system_error(qls_errc::invalid_verification);

    {
        std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

        auto iter = m_impl->m_friendRoomVerification_map.find(
            { sender, receiver });
        if (iter == m_impl->m_friendRoomVerification_map.cend())
            throw std::system_error(qls_errc::verification_not_existed);

        iter->second = true;
    }

    GroupID group_id = serverManager.addPrivateRoom(sender, receiver);
    // update the 1st user's friend list
    {
        auto ptr = serverManager.getUser(sender);
        ptr->updateFriendList(
            [receiver](std::unordered_set<qls::UserID>& set){
            set.insert(receiver);
        });
    }
    
    // update the 2nd user's friend list
    {
        auto ptr = serverManager.getUser(receiver);
        ptr->updateFriendList(
            [sender](std::unordered_set<qls::UserID>& set){
            set.insert(sender);
        });
    }

    this->removeFriendRoomVerification(sender, receiver);
}

void VerificationManager::rejectFriendVerification(UserID sender, UserID receiver)
{
    if (sender == receiver)
        throw std::system_error(qls_errc::invalid_verification);

    this->removeFriendRoomVerification(sender, receiver);
}

bool VerificationManager::isFriendVerified(UserID sender, UserID receiver) const
{
    if (sender == receiver)
        throw std::system_error(qls_errc::invalid_verification);

    std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

    auto iter = m_impl->m_friendRoomVerification_map.find(
        { sender, receiver });
    if (iter == m_impl->m_friendRoomVerification_map.cend())
        throw std::system_error(qls_errc::verification_not_existed);

    return iter->second;
}

void VerificationManager::removeFriendRoomVerification(UserID sender, UserID receiver)
{
    if (sender == receiver)
        throw std::system_error(qls_errc::invalid_verification);

    {
        std::unique_lock lock(m_impl->m_friendRoomVerification_map_mutex);

        auto iter = m_impl->m_friendRoomVerification_map.find(
            { sender, receiver });
        if (iter == m_impl->m_friendRoomVerification_map.cend())
            return;

        m_impl->m_friendRoomVerification_map.erase(iter);
    }

    serverManager.getUser(sender)
        ->removeFriendVerification(receiver);
    serverManager.getUser(receiver)
        ->removeFriendVerification(sender);
}

void VerificationManager::applyGroupRoomVerification(UserID sender, GroupID receiver)
{
    if (!serverManager.hasGroupRoom(receiver))
            throw std::system_error(qls_errc::group_room_not_existed);
    if (!serverManager.hasUser(sender))
            throw std::system_error(qls_errc::user_not_existed);

    {
        std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

        if (m_impl->m_groupVerification_map.find(
            { sender, receiver })
            != m_impl->m_groupVerification_map.cend())
            throw std::system_error(qls_errc::verification_not_existed);

        m_impl->m_groupVerification_map.emplace(
            GroupVerification{sender, receiver},
            false);
    }

    {
        qls::Verification::GroupVerification uv;

        uv.group_id = receiver;
        uv.user_id = sender;
        uv.verification_type = qls::Verification::Sent;

        auto ptr = serverManager.getUser(sender);
        ptr->addGroupVerification(receiver, std::move(uv));
    }

    // Only modify the administrator's verification list
    // Because of over consumption of computer power
    {
        qls::Verification::GroupVerification uv;

        uv.group_id = receiver;
        uv.user_id = sender;
        uv.verification_type = qls::Verification::Received;

        UserID adminID = serverManager.getGroupRoom(receiver)->getAdministrator();
        auto ptr = serverManager.getUser(adminID);
        ptr->addGroupVerification(receiver, std::move(uv));
    }
}

bool VerificationManager::hasGroupRoomVerification(UserID sender, GroupID receiver) const
{
    std::shared_lock lock(m_impl->m_groupVerification_map_mutex);

    return m_impl->m_groupVerification_map.find(
        { sender, receiver }) !=
        m_impl->m_groupVerification_map.cend();
}

void VerificationManager::acceptGroupRoom(UserID sender, GroupID receiver)
{
    {
        std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

        auto iter = m_impl->m_groupVerification_map.find(
            { sender, receiver });
        if (iter == m_impl->m_groupVerification_map.cend())
            throw std::system_error(qls_errc::verification_not_existed);

        iter->second = true;
    }

    bool _ = serverManager.getGroupRoom(receiver)->addMember(sender);
    // update user's list
    auto ptr = serverManager.getUser(sender);
    ptr->updateGroupList([receiver](std::unordered_set<qls::GroupID>& set){
        set.insert(receiver);
    });
    this->removeGroupRoomVerification(sender, receiver);
}

void VerificationManager::rejectGroupRoom(UserID sender, GroupID receiver)
{
    this->removeGroupRoomVerification(sender, receiver);
}

bool VerificationManager::isGroupRoomVerified(UserID sender, GroupID receiver) const
{
    std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

    auto iter = m_impl->m_groupVerification_map.find(
        { sender, receiver });
    if (iter == m_impl->m_groupVerification_map.cend())
        throw std::system_error(qls_errc::verification_not_existed);

    return iter->second;
}

void VerificationManager::removeGroupRoomVerification(UserID sender, GroupID receiver)
{
    {
        std::unique_lock lock(m_impl->m_groupVerification_map_mutex);

        auto iter = m_impl->m_groupVerification_map.find(
            { sender, receiver });
        if (iter == m_impl->m_groupVerification_map.cend())
            throw std::system_error(qls_errc::verification_not_existed);

        m_impl->m_groupVerification_map.erase(iter);
    }
    UserID adminID = serverManager.getGroupRoom(receiver)->getAdministrator();
    serverManager.getUser(adminID)
        ->removeGroupVerification(receiver, sender);
    serverManager.getUser(sender)->removeGroupVerification(receiver, sender);
}

} // namespace qls
