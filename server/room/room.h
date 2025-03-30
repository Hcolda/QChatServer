#ifndef ROOM_H
#define ROOM_H

#include <memory>
#include <functional>
#include <stdexcept>
#include <string_view>
#include <memory_resource>

#include "userid.hpp"
#include "user.h"

namespace qls
{

enum class MessageType
{
    NOMAL_MESSAGE = 0,
    TIP_MESSAGE
};

struct MessageStructure
{
    UserID sender = UserID(-1ll);
    std::string message;
    MessageType type;
    UserID receiver = UserID(-1ll);
};

struct MessageResult
{
    std::chrono::utc_clock::time_point  time_point;
    MessageStructure                    message_struct;
};

class RoomInterface
{
public:
    virtual ~RoomInterface() noexcept = default;

    virtual void joinRoom(UserID user_id) = 0;
    [[nodiscard]] virtual bool hasUser(UserID user_id) const = 0;
    virtual void leaveRoom(UserID user_id) = 0;

    virtual void sendData(std::string_view data) = 0;
    virtual void sendData(std::string_view data, UserID user_id) = 0;
};

struct TCPRoomImpl;
struct TCPRoomImplDeleter
{
    std::pmr::memory_resource* memory_resource;
    void operator()(TCPRoomImpl*) noexcept;
};

class TCPRoom: public RoomInterface
{
public:
    TCPRoom(std::pmr::memory_resource* mr);
    TCPRoom(const TCPRoom&) = delete;
    TCPRoom(TCPRoom&&) = delete;
    virtual ~TCPRoom() noexcept;

    TCPRoom& operator=(const TCPRoom&) = delete;
    TCPRoom& operator=(TCPRoom&&) = delete;

    virtual void joinRoom(UserID user_id);
    [[nodiscard]] virtual bool hasUser(UserID user_id) const;
    virtual void leaveRoom(UserID user_id);

    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);

private:
    std::unique_ptr<TCPRoomImpl, TCPRoomImplDeleter> m_impl;
};

struct KCPRoomImpl;
struct KCPRoomImplDeleter
{
    std::pmr::memory_resource* memory_resource;
    void operator()(KCPRoomImpl*) noexcept;
};

class KCPRoom: public RoomInterface
{
public:
    KCPRoom(std::pmr::memory_resource* mr);
    KCPRoom(const KCPRoom&) = delete;
    KCPRoom(KCPRoom&&) = delete;
    virtual ~KCPRoom() noexcept;

    KCPRoom& operator=(const KCPRoom&) = delete;
    KCPRoom& operator=(KCPRoom&&) = delete;

    virtual void joinRoom(UserID user_id);
    [[nodiscard]] virtual bool hasUser(UserID user_id) const;
    virtual void leaveRoom(UserID user_id);

    virtual void addSocket(const std::shared_ptr<KCPSocket>& socket);
    [[nodiscard]] virtual bool hasSocket(const std::shared_ptr<KCPSocket>& socket) const;
    virtual void removeSocket(const std::shared_ptr<KCPSocket>& socket);

    virtual void sendData(std::string_view data);
    [[deprecated("This function is not useful at kcp connection")]]
        virtual void sendData(std::string_view data, UserID user_id);

private:
    std::unique_ptr<KCPRoomImpl, KCPRoomImplDeleter> m_impl;
};

class TextDataRoom: public TCPRoom
{
public:
    TextDataRoom(std::pmr::memory_resource* mr):
        TCPRoom(mr) {}
    virtual ~TextDataRoom() noexcept = default;

protected:
    virtual void sendData(std::string_view data);
    virtual void sendData(std::string_view data, UserID user_id);
};

} // namespace qls

#endif // !ROOM_H
