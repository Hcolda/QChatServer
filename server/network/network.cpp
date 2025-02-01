#include "network.h"

#include <memory_resource>
#include <asio/experimental/awaitable_operators.hpp>
#include <logger.hpp>
#include <system_error>
#include <Json.h>
#include <Ini.h>

#include "socketFunctions.h"
#include "definition.hpp"
#include "socket.h"
#include "manager.h"
#include "qls_error.h"
#include "connection.hpp"

extern Log::Logger serverLogger;
extern qls::Network serverNetwork;
extern qls::Manager serverManager;
extern qls::SocketFunction serverSocketFunction;
extern qini::INIObject serverIni;

using asio::ip::tcp;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
namespace this_coro = asio::this_coro;
using namespace asio;

qls::Network::Network() :
    m_port(55555),
    m_thread_num((12 > static_cast<int>(std::thread::hardware_concurrency())
        ? 12 : static_cast<int>(std::thread::hardware_concurrency())))
    {
        m_threads = std::make_unique<std::thread[]>(static_cast<std::size_t>(m_thread_num));
    }

qls::Network::~Network()
{
    for (int i = 0; i < m_thread_num; i++) {
        if (m_threads[i].joinable())
            m_threads[i].join();
    }
}

void qls::Network::setTlsConfig(
    std::function<std::shared_ptr<
        ssl::context>()> callback_handle)
{
    if (!callback_handle)
        throw std::system_error(qls_errc::null_tls_callback_handle);
    m_ssl_context_ptr = callback_handle();
    if (!m_ssl_context_ptr)
        throw std::system_error(qls_errc::null_tls_context);
}

void qls::Network::run(std::string_view host, unsigned short port)
{
    m_host = host;
    m_port = port;

    // Check if SSL context ptr is null
    if (!m_ssl_context_ptr)
        throw std::system_error(qls_errc::null_tls_context);

    try {
        signal_set signals(m_io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { m_io_context.stop(); });

        co_spawn(m_io_context, listener(), detached);
        co_spawn(m_io_context, m_rateLimiter.auto_clean(), detached);
        for (int i = 0; i < m_thread_num; i++) {
            m_threads[i] = std::thread([&]() {
                m_io_context.run();
                });
        }
        for (int i = 0; i < m_thread_num; i++) {
            if (m_threads[i].joinable())
                m_threads[i].join();
        }
    } catch (const std::exception& e) {
        serverLogger.error(ERROR_WITH_STACKTRACE(e.what()));
    }
}

void qls::Network::stop()
{
    m_io_context.stop();
}

static std::pmr::synchronized_pool_resource socket_sync_pool;

awaitable<void> qls::Network::echo(ip::tcp::socket origin_socket)
{
    auto executor = co_await this_coro::executor;

    // Check socket
    if (!m_rateLimiter.allow_connection(origin_socket.remote_endpoint().address())) {
        std::error_code ec;
        origin_socket.close(ec);
        // serverLogger.warning('[', origin_socket.remote_endpoint().address().to_string(), "] is seemly attacking the server!");
        co_return;
    }

    // Load SSL socket pointer
    std::shared_ptr<Connection> connection_ptr = std::allocate_shared<Connection>(
        std::pmr::polymorphic_allocator<Connection>(&socket_sync_pool),
        std::move(origin_socket), *m_ssl_context_ptr);
    // String address for data processing
    std::string addr = socket2ip(connection_ptr->socket);
    // Socket encrypted structure
    std::shared_ptr<SocketDataStructure> sds = std::make_shared<SocketDataStructure>();
    // Register the socket
    serverManager.registerConnection(connection_ptr);

    try {
        serverLogger.info(std::format("[{}] connected to the server", addr));

        // SSL handshake
        co_await connection_ptr->socket.async_handshake(ssl::stream_base::server, use_awaitable);

        char data[8192] {0};
        for (;;) {
            do {
                std::size_t n = co_await connection_ptr->socket.async_read_some(buffer(data),
                    bind_executor(connection_ptr->strand, use_awaitable));
                // serverLogger.info((std::format("[{}] received message: {}", addr, showBinaryData({data, n}))));
                sds->package.write({ data, n });
            } while (!sds->package.canRead());

            while (sds->package.canRead()) {
                std::shared_ptr<qls::DataPackage> datapack;

                // Check if the data package is normal
                try {
                    // Data package
                    datapack = std::shared_ptr<qls::DataPackage>(
                        qls::DataPackage::stringToPackage(
                            sds->package.read()));
                    if (datapack->type == DataPackage::HeartBeat) continue;
                    if (datapack->getData() != "test")
                        throw std::system_error(qls_errc::connection_test_failed);
                } catch (const std::exception& e) {
                    serverLogger.error("[", addr, "]", ERROR_WITH_STACKTRACE(e.what()));
                    std::error_code ec;
                    connection_ptr->socket.shutdown(ec);
                    co_return;
                }

                auto executeFunction = [addr](std::shared_ptr<Connection> connection_ptr,
                    std::shared_ptr<Network::SocketDataStructure> sds) -> awaitable<void> {
                    using namespace experimental::awaitable_operators;
                    // Create a watchdog
                    auto watchdog = [addr](std::chrono::steady_clock::time_point & deadline) -> awaitable<void> {
                        steady_timer timer(co_await this_coro::executor);
                        auto now = std::chrono::steady_clock::now();
                        while (deadline > now) {
                            timer.expires_at(deadline);
                            co_await timer.async_wait(use_awaitable);
                            now = std::chrono::steady_clock::now();
                        }
                        throw std::system_error(make_error_code(std::errc::timed_out));
                    };
                    std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
                    try {
                        co_await (SocketService::echo(connection_ptr, std::move(sds), deadline) && watchdog(deadline));
                    } catch (const std::system_error& e) {
                        const auto& errc = e.code();
                        if (errc.message() == "End of file")
                            serverLogger.info(std::format("[{}] disconnected from the server", addr));
                        else
                            serverLogger.error('[', errc.category().name(), ']', errc.message());
                    } catch (const std::exception& e) {
                        serverLogger.error(std::string(e.what()));
                    }
                    serverManager.removeConnection(connection_ptr);
                    co_return;
                    };

                co_spawn(executor, executeFunction(std::move(connection_ptr), std::move(sds)), detached);
                co_return;
            }
        }
    }
    catch (const std::system_error& e) {
        const auto& errc = e.code();
        if (errc.message() == "End of file")
            serverLogger.info(std::format("[{}] disconnected from the server", addr));
        else
            serverLogger.error('[', errc.category().name(), ']', errc.message());
    }
    catch (const std::exception& e) {
        serverLogger.error(ERROR_WITH_STACKTRACE(e.what()));
    }
    serverManager.removeConnection(connection_ptr);
    co_return;
}

awaitable<void> qls::Network::listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { ip::make_address(m_host), m_port });

    // SYN anti-attack & Dos anti-attack
    acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
    acceptor.set_option(socket_base::receive_buffer_size(1024*1024));
    acceptor.set_option(tcp::acceptor::enable_connection_aborted(true));
#if defined(__LINUX__) || defined(__UNIX__)
    int fd = acceptor.native_handle();
    int syncnt = 2;
    setsockopt(fd, IPPROTO_TCP, TCP_SYNCNT, &syncnt, sizeof(syncnt));
    
    int cookie = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_SYNCOOKIE, &cookie, sizeof(cookie));
#endif
    for (;;) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

inline std::string qls::socket2ip(const qls::Socket& s)
{
    auto ep = s.lowest_layer().remote_endpoint();
    return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
}

inline std::string qls::showBinaryData(std::string_view data)
{
    auto isShowableCharacter = [](unsigned char ch) -> bool {
        return 32 <= ch && ch <= 126;
        };

    std::string result;
    for (const auto& i : data) {
        if (isShowableCharacter(static_cast<unsigned char>(i)))
            result += i;
        else {
            std::string hex;
            int locch = static_cast<unsigned char>(i);
            while (locch) {
                if (locch % 16 < 10) {
                    hex += ('0' + (locch % 16));
                    locch /= 16;
                    continue;
                }
                switch (locch % 16) {
                case 10:
                    hex += 'a';
                    break;
                case 11:
                    hex += 'b';
                    break;
                case 12:
                    hex += 'c';
                    break;
                case 13:
                    hex += 'd';
                    break;
                case 14:
                    hex += 'e';
                    break;
                case 15:
                    hex += 'f';
                    break;
                }
                locch /= 16;
            }

            if (hex.empty())
                result += "\\x00";
            else if (hex.size() == 1)
                result += "\\x0" + hex;
            else
                result += "\\x" + hex;
        }
    }

    return result;
}
