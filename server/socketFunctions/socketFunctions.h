#ifndef SOCKET_FUNCTIONS_H
#define SOCKET_FUNCTIONS_H

#include <asio.hpp>
#include <memory>

#include "connection.hpp"
#include "dataPackage.hpp"

namespace qls {

struct SocketServiceImpl;

class SocketService final {
public:
  SocketService(
      const std::shared_ptr<Connection<asio::ip::tcp::socket>> &connection_ptr);
  ~SocketService() noexcept;

  /**
   * @brief Get the socket pointer
   * @return Connection pointer
   */
  std::shared_ptr<Connection<asio::ip::tcp::socket>> get_connection_ptr() const;

  /**
   * @brief Process function
   * @param connection_ptr Connection pointer
   * @param data Decrypted data
   * @param pack Original data packet
   */
  asio::awaitable<void> process(std::string_view data, DataPackagePtr pack);

private:
  std::unique_ptr<SocketServiceImpl> m_impl;
};

} // namespace qls

#endif // !SOCKET_FUNCTIONS_H
