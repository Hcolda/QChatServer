#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <asio.hpp>
#include <asio/ssl/stream.hpp>

namespace qls {

template <class T> struct Connection {
  // Socket used to send and receive data
  asio::ssl::stream<T> socket;
  // Keep the sending and receiving data thread-safe
  // (but must be kept by hand)
  // E.g: asio::async_write(socket, asio::buffer(data),
  // asio::bind_executor(strand, token))
  asio::strand<asio::any_io_executor> strand;

  template <class U>
  Connection(U &&lsocket, asio::ssl::context &context)
      : socket(std::forward<U>(lsocket), context),
        strand(asio::make_strand(socket.get_executor())) {}

  ~Connection() noexcept {
    std::error_code errorc;
    errorc = socket.shutdown(errorc);
  }
};

} // namespace qls

#endif // !CONNECTION_HPP
