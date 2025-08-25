#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <memory_resource>
#include <string>
#include <thread>

#include "rateLimiter.hpp"
#include "socket.hpp"
#include "string_param.hpp"

namespace qls {

/**
 * @brief Converts a socket's address to a string representation.
 * @param s The socket.
 * @return The string representation of the socket's address.
 */
inline std::string socket2ip(const qls::Socket &s);

/**
 * @brief Displays binary data as a string.
 * @param data The binary data.
 * @return The string representation of the binary data.
 */
inline std::string showBinaryData(string_param data);

/**
 * @class Network
 * @brief Manages network operations including connection handling and data
 * transmission.
 */
class Network final {
public:
  Network(std::pmr::memory_resource *memory_resource);
  Network(const Network &) = delete;
  Network(Network &&) = delete;
  ~Network();

  constexpr static std::uint8_t thread_num = 12;
  constexpr static std::uint16_t port_num = 55555;
  constexpr static std::chrono::seconds timeout_num = std::chrono::seconds(60);
  constexpr static std::chrono::seconds heart_beat_check_interval =
      std::chrono::seconds(10);
  constexpr static std::uint32_t max_heart_beat_num = 10;
  constexpr static std::uint32_t buffer_lenth = 8192;

  /**
   * @brief Sets the TLS configuration.
   * @param callback_handle A callback function to configure TLS.
   */
  void setTlsConfig(const std::function<std::shared_ptr<asio::ssl::context>()>
                        &callback_handle);

  /**
   * @brief Runs the network.
   * @param host The host address.
   * @param port The port number.
   */
  void run(string_param host, std::uint16_t port);

  /**
   * @brief Stops the network operations.
   */
  void stop();

  /**
   * @brief Gets the io_context for this network
   */
  [[nodiscard]] asio::io_context &get_io_context() noexcept;

private:
  asio::awaitable<void> process(asio::ip::tcp::socket socket);
  asio::awaitable<void> listener();
  static asio::awaitable<void>
  timeout(const std::chrono::nanoseconds &duration);

  std::string m_host;    ///< Host address.
  unsigned short m_port; ///< Port number.
  std::unique_ptr<std::thread[]>
      m_threads;                    ///< Thread pool for handling connections.
  const std::uint32_t m_thread_num; ///< Number of threads.
  asio::io_context m_io_context;    ///< IO context for ASIO.
  std::shared_ptr<asio::ssl::context>
      m_ssl_context_ptr; ///< Shared pointer to the SSL context.
  RateLimiter m_rateLimiter;

  std::pmr::memory_resource *m_memory_resource;
};

} // namespace qls

#endif // !NETWORK_HPP
