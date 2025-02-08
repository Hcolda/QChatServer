#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <thread>
#include <chrono>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <string>
#include <memory>

#include "definition.hpp"
#include "package.h"
#include "dataPackage.h"
#include "socket.h"
#include "rateLimiter.hpp"

namespace qls
{

/**
 * @brief Converts a socket's address to a string representation.
 * @param s The socket.
 * @return The string representation of the socket's address.
 */
inline std::string socket2ip(const qls::Socket& s);

/**
 * @brief Displays binary data as a string.
 * @param data The binary data.
 * @return The string representation of the binary data.
 */
inline std::string showBinaryData(std::string_view data);

/**
 * @class Network
 * @brief Manages network operations including connection handling and data transmission.
 */
class Network final
{
public:
    Network();
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    ~Network();

    /**
     * @brief Sets the TLS configuration.
     * @param callback_handle A callback function to configure TLS.
     */
    void setTlsConfig(
        std::function<std::shared_ptr<asio::ssl::context>()> callback_handle);

    /**
     * @brief Runs the network.
     * @param host The host address.
     * @param port The port number.
     */
    void run(std::string_view host, unsigned short port);

    /**
     * @brief Stops the network operations.
     */
    void stop();

private:
    /**
     * @brief Handles echo functionality for a socket.
     * @param socket The socket.
     * @return An awaitable task.
     */
    asio::awaitable<void> echo(asio::ip::tcp::socket socket);

    /**
     * @brief Listens for incoming connections.
     * @return An awaitable task.
     */
    asio::awaitable<void> listener();

    std::string                         m_host; ///< Host address.
    unsigned short                      m_port; ///< Port number.
    std::unique_ptr<std::thread[]>      m_threads; ///< Thread pool for handling connections.
    const int                           m_thread_num; ///< Number of threads.
    asio::io_context                    m_io_context; ///< IO context for ASIO.
    std::shared_ptr<asio::ssl::context> m_ssl_context_ptr; ///< Shared pointer to the SSL context.
    RateLimiter                         m_rateLimiter;
};

} // namespace qls

#endif // !NETWORK_HPP
