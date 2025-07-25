#ifndef PRIVATE_ROOM_H
#define PRIVATE_ROOM_H

#include <chrono>
#include <memory>
#include <memory_resource>
#include <string_view>

#include "room.h"
#include "userid.hpp"

namespace qls {

struct PrivateRoomImpl;
struct PrivateRoomImplDeleter {
  std::pmr::memory_resource *memory_resource;
  void operator()(PrivateRoomImpl *pri) noexcept;
};

/*
 * @brief 私聊房间基类
 */
class PrivateRoom : public TextDataRoom {
public:
  PrivateRoom(const UserID &user_id_1, const UserID &user_id_2, bool is_create,
              std::pmr::memory_resource *memory_resouce);
  PrivateRoom(const PrivateRoom &) = delete;
  PrivateRoom(PrivateRoom &&) = delete;

  ~PrivateRoom() noexcept;

  void sendMessage(std::string_view message, const UserID &sender_user_id);
  void sendTipMessage(std::string_view message, const UserID &sender_user_id);
  std::vector<MessageResult>
  getMessage(const std::chrono::utc_clock::time_point &from,
             const std::chrono::utc_clock::time_point &to);

  std::pair<UserID, UserID> getUserID() const;
  bool hasMember(const UserID &user_id) const;

  void removeThisRoom();
  bool canBeUsed() const;

  asio::awaitable<void> auto_clean();
  void stop_cleaning();

private:
  std::unique_ptr<PrivateRoomImpl, PrivateRoomImplDeleter> m_impl;
};

} // namespace qls

#endif // !PRIVATE_ROOM_H
