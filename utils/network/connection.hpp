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
  Connection(U &&s, asio::ssl::context &context)
      : socket(std::forward<U>(s), context),
        strand(asio::make_strand(s.get_executor())) {}

  ~Connection() noexcept {
    std::error_code ec;
    ec = socket.shutdown(ec);
  }
};

} // namespace qls

#endif // !CONNECTION_HPP
