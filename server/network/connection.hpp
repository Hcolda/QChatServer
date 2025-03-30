#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <asio.hpp>
#include "socket.hpp"

namespace qls
{

struct Connection
{
    // Socket used to send and receive data
    Socket socket;
    // Keep the sending and receiving data thread-safe
    // (but must be kept by hand)
    // E.g: asio::async_write(socket, asio::buffer(data), asio::bind_executor(strand, token))
    asio::strand<asio::any_io_executor> strand;

    Connection(asio::ip::tcp::socket s, asio::ssl::context& context):
        socket(std::move(s), context),
        strand(asio::make_strand(s.get_executor())) {}

    ~Connection()
    {
        [[maybe_unused]] std::error_code ec;
        socket.shutdown(ec);
    }
};

} // namespace qls


#endif // !CONNECTION_HPP
