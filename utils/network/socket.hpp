#ifndef SOCKET_H
#define SOCKET_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <kcp.hpp>

namespace qls {
using Socket = asio::ssl::stream<asio::ip::tcp::socket>;
using KCPSocket = asio::ssl::stream<moon::kcp::connection>;
} // namespace qls

#endif // !SOCKET_H
