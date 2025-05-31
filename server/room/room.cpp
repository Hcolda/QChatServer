#include "room.h"

#include <memory>
#include <memory_resource>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#include "dataPackage.h"
#include "logger.hpp"
#include "manager.h"
#include "user.h"

extern qls::Manager serverManager;
extern Log::Logger serverLogger;

namespace qls {

/*
 * ------------------------------------------------------------------------
 * class TCPRoom
 * ------------------------------------------------------------------------
 */

struct TCPRoomImpl {
  TCPRoomImpl(std::pmr::memory_resource *res) : m_user_map(res) {}

  std::pmr::unordered_map<UserID, std::weak_ptr<User>> m_user_map;
  mutable std::shared_mutex m_user_map_mutex;
};

void TCPRoomImplDeleter::operator()(TCPRoomImpl *mem_pointer) const noexcept {
  std::pmr::polymorphic_allocator<TCPRoomImpl>(memory_resource)
      .delete_object(mem_pointer);
}

TCPRoom::TCPRoom(std::pmr::memory_resource *res)
    : m_impl(
          std::pmr::polymorphic_allocator<>(res).new_object<TCPRoomImpl>(res)) {
}

TCPRoom::~TCPRoom() noexcept = default;

void TCPRoom::joinRoom(UserID user_id) {
  std::unique_lock lock(m_impl->m_user_map_mutex);
  if (m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend()) {
    return;
  }

  m_impl->m_user_map.emplace(user_id, serverManager.getUser(user_id));
}

bool TCPRoom::hasUser(UserID user_id) const {
  std::shared_lock lock(m_impl->m_user_map_mutex);
  return m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend();
}

void TCPRoom::leaveRoom(UserID user_id) {
  std::unique_lock lock(m_impl->m_user_map_mutex);
  auto iter = m_impl->m_user_map.find(user_id);
  if (iter == m_impl->m_user_map.cend()) {
    return;
  }

  m_impl->m_user_map.erase(iter);
}

void TCPRoom::sendData(std::string_view data) {
  std::shared_lock lock(m_impl->m_user_map_mutex);

  for (const auto &[user_id, user_ptr] : std::as_const(m_impl->m_user_map)) {
    if (!user_ptr.expired()) {
      user_ptr.lock()->notifyAll(data);
    }
  }
}

void TCPRoom::sendData(std::string_view data, UserID user_id) {
  std::shared_lock lock(m_impl->m_user_map_mutex);
  if (m_impl->m_user_map.find(user_id) == m_impl->m_user_map.cend()) {
    throw std::logic_error("User id not in room.");
  }
  serverManager.getUser(user_id)->notifyAll(data);
}

/*
 * ------------------------------------------------------------------------
 * class KCPRoom
 * ------------------------------------------------------------------------
 */

struct KCPRoomImpl {
  KCPRoomImpl(std::pmr::memory_resource *res)
      : m_user_map(res), m_socket_map(res) {}

  std::pmr::unordered_map<UserID, std::weak_ptr<User>> m_user_map;
  mutable std::shared_mutex m_user_map_mutex;

  std::pmr::unordered_set<std::shared_ptr<KCPSocket>> m_socket_map;
  mutable std::shared_mutex m_socket_map_mutex;
};

KCPRoom::KCPRoom(std::pmr::memory_resource *res)
    : m_impl(
          std::pmr::polymorphic_allocator<>(res).new_object<KCPRoomImpl>(res)) {
}

void KCPRoomImplDeleter::operator()(KCPRoomImpl *mem_pointer) const noexcept {
  std::pmr::polymorphic_allocator<KCPRoomImpl>(memory_resource)
      .delete_object(mem_pointer);
}

KCPRoom::~KCPRoom() noexcept = default;

void KCPRoom::joinRoom(UserID user_id) {
  std::unique_lock lock(m_impl->m_user_map_mutex);
  if (m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend()) {
    return;
  }

  m_impl->m_user_map.emplace(user_id, serverManager.getUser(user_id));
}

bool KCPRoom::hasUser(UserID user_id) const {
  std::shared_lock lock(m_impl->m_user_map_mutex);
  return m_impl->m_user_map.find(user_id) != m_impl->m_user_map.cend();
}

void KCPRoom::leaveRoom(UserID user_id) {
  std::unique_lock lock(m_impl->m_user_map_mutex);
  auto iter = m_impl->m_user_map.find(user_id);
  if (iter == m_impl->m_user_map.cend()) {
    return;
  }

  m_impl->m_user_map.erase(iter);
}

void KCPRoom::addSocket(const std::shared_ptr<KCPSocket> &socket) {
  std::lock_guard<std::shared_mutex> lock(m_impl->m_socket_map_mutex);
  m_impl->m_socket_map.emplace(socket);
}

bool KCPRoom::hasSocket(const std::shared_ptr<KCPSocket> &socket) const {
  std::shared_lock lock(m_impl->m_socket_map_mutex);
  return m_impl->m_socket_map.find(socket) != m_impl->m_socket_map.cend();
}

void KCPRoom::removeSocket(const std::shared_ptr<KCPSocket> &socket) {
  std::lock_guard<std::shared_mutex> lock(m_impl->m_socket_map_mutex);
  auto iter = m_impl->m_socket_map.find(socket);
  if (iter != m_impl->m_socket_map.end()) {
    m_impl->m_socket_map.erase(iter);
  }
}

void KCPRoom::sendData(std::string_view data) {
  std::shared_lock lock(m_impl->m_socket_map_mutex);
  for (const auto &socket : std::as_const(m_impl->m_socket_map)) {
    socket->async_write_some(asio::buffer(data), [](auto, auto) {});
  }
}

void KCPRoom::sendData(std::string_view data, UserID user_id) {}

/*
 * ------------------------------------------------------------------------
 * class TextDataRoom
 * ------------------------------------------------------------------------
 */

void TextDataRoom::sendData(std::string_view data) {
  auto package = DataPackage::makePackage(data);
  package->type = DataPackage::Text;
  TCPRoom::sendData(package->packageToString());
}

void TextDataRoom::sendData(std::string_view data, UserID user_id) {
  auto package = DataPackage::makePackage(data);
  package->type = DataPackage::Text;
  TCPRoom::sendData(package->packageToString(), user_id);
}

} // namespace qls
