#include "privateRoom.h"

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <memory_resource>
#include <shared_mutex>
#include <system_error>

#include <Json.h>

#include "manager.h"
#include "qls_error.h"

extern qls::Manager serverManager;

namespace qls {

struct PrivateRoomImpl {
  const UserID m_user_id_1, m_user_id_2;

  std::atomic<bool> m_can_be_used;

  std::pmr::map<std::chrono::utc_clock::time_point, MessageStructure>
      m_message_map;
  std::shared_mutex m_message_map_mutex;

  asio::steady_timer m_clear_timer;

  std::pmr::memory_resource *m_local_memory_resouce;

  PrivateRoomImpl(const UserID &user_id_1, const UserID &user_id_2,
                  std::pmr::memory_resource *memory_resouce)
      : m_user_id_1(user_id_1), m_user_id_2(user_id_2),
        m_local_memory_resouce(memory_resouce), m_message_map(memory_resouce),
        m_clear_timer(serverManager.getServerNetwork().get_io_context()) {}
};

void PrivateRoomImplDeleter::operator()(PrivateRoomImpl *pri) noexcept {
  std::pmr::polymorphic_allocator<>(memory_resource).delete_object(pri);
}

// PrivateRoom
PrivateRoom::PrivateRoom(const UserID &user_id_1, const UserID &user_id_2,
                         bool is_create,
                         std::pmr::memory_resource *memory_resouce)
    : TextDataRoom(memory_resouce),
      m_impl(std::pmr::polymorphic_allocator<>(memory_resouce)
                 .new_object<PrivateRoomImpl>(user_id_1, user_id_2,
                                              memory_resouce),
             {memory_resouce}) {
  // if (is_create) {
  //   // sql 创建private room
  //   m_impl->m_can_be_used = true;
  // } else {
  //   // sql 读取private room
  //   m_impl->m_can_be_used = true;
  // }
  m_impl->m_can_be_used = true;

  TextDataRoom::joinRoom(user_id_1);
  TextDataRoom::joinRoom(user_id_2);
  asio::co_spawn(serverManager.getServerNetwork().get_io_context(),
                 auto_clean(), asio::detached);
}

PrivateRoom::~PrivateRoom() noexcept { stop_cleaning(); }

void PrivateRoom::sendMessage(string_param message,
                              const UserID &sender_user_id) {
  if (!m_impl->m_can_be_used) {
    throw std::system_error(
        make_error_code(qls_errc::private_room_unable_to_use));
  }
  if (!hasUser(sender_user_id)) {
    return;
  }

  // 存储数据
  {
    std::unique_lock lock(m_impl->m_message_map_mutex);
    m_impl->m_message_map.insert(
        {std::chrono::utc_clock::now(),
         {sender_user_id, std::string(message), MessageType::TIP_MESSAGE}});
  }

  qjson::JObject json;
  json["type"] = "private_message";
  json["data"]["user_id"] = sender_user_id.getOriginValue();
  json["data"]["message"] = message;

  sendData(json.to_string());
}

void PrivateRoom::sendTipMessage(string_param message,
                                 const UserID &sender_user_id) {
  if (!m_impl->m_can_be_used) {
    throw std::system_error(
        make_error_code(qls_errc::private_room_unable_to_use));
  }
  if (!hasUser(sender_user_id)) {
    return;
  }

  // 存储数据
  {
    std::unique_lock lock(m_impl->m_message_map_mutex);
    m_impl->m_message_map.insert(
        {std::chrono::utc_clock::now(),
         {sender_user_id, std::string(message), MessageType::TIP_MESSAGE}});
  }

  qjson::JObject json;
  json["type"] = "private_tip_message";
  json["data"]["user_id"] = sender_user_id.getOriginValue();
  json["data"]["message"] = message;

  sendData(json.to_string());
}

std::vector<MessageResult>
PrivateRoom::getMessage(const std::chrono::utc_clock::time_point &from,
                        const std::chrono::utc_clock::time_point &to) {
  if (!m_impl->m_can_be_used) {
    throw std::system_error(
        make_error_code(qls_errc::group_room_unable_to_use));
  }
  if (from > to) {
    return {};
  }

  std::shared_lock lock(m_impl->m_message_map_mutex);
  const auto &message_map = std::as_const(m_impl->m_message_map);
  if (message_map.empty()) {
    return {};
  }
  auto start = message_map.lower_bound(from);
  auto end = message_map.upper_bound(to);
  std::vector<MessageResult> revec;
  for (; start != end; ++start) {
    revec.emplace_back(start->first, start->second);
  }
  return revec;
}

std::pair<UserID, UserID> PrivateRoom::getUserID() const {
  if (!m_impl->m_can_be_used) {
    throw std::system_error(
        make_error_code(qls_errc::private_room_unable_to_use));
  }
  return {m_impl->m_user_id_1, m_impl->m_user_id_2};
}

bool PrivateRoom::hasMember(const UserID &user_id) const {
  if (!m_impl->m_can_be_used) {
    throw std::system_error(
        make_error_code(qls_errc::private_room_unable_to_use));
  }
  return user_id == m_impl->m_user_id_1 || user_id == m_impl->m_user_id_2;
}

void PrivateRoom::removeThisRoom() {
  m_impl->m_can_be_used = false;

  {
    // sql 上面删除此房间
  }

  // 剩下其他东西
}

bool PrivateRoom::canBeUsed() const { return m_impl->m_can_be_used; }

asio::awaitable<void> PrivateRoom::auto_clean() {
  using namespace std::chrono_literals;
  try {
    while (true) {
      m_impl->m_clear_timer.expires_after(10min);
      co_await m_impl->m_clear_timer.async_wait(asio::use_awaitable);
      std::unique_lock lock(m_impl->m_message_map_mutex);
      auto end = m_impl->m_message_map.upper_bound(
          std::chrono::utc_clock::now() - std::chrono::weeks(1));
      m_impl->m_message_map.erase(m_impl->m_message_map.begin(), end);
    }
  } catch (...) {
    co_return;
  }
}

void PrivateRoom::stop_cleaning() {
  std::error_code errorc;
  m_impl->m_clear_timer.cancel(errorc);
}

} // namespace qls
