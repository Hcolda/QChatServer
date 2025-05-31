#include "socketFunctions.h"

#include <Json.h>
#include <asio/experimental/awaitable_operators.hpp>
#include <logger.hpp>
#include <system_error>

#include "JsonMsgProcess.h"
#include "dataPackage.h"
#include "manager.h"
#include "qls_error.h"
#include "returnStateMessage.hpp"
#include "userid.hpp"

extern Log::Logger serverLogger;
extern qls::Manager serverManager;

// SocketService
namespace qls {
struct SocketServiceImpl {
  // socket ptr
  std::shared_ptr<Connection<asio::ip::tcp::socket>> m_connection_ptr;
  // JsonMsgProcess
  JsonMessageProcess m_jsonProcess;
};

SocketService::SocketService(
    std::shared_ptr<Connection<asio::ip::tcp::socket>> connection_ptr)
    : m_impl(std::make_unique<SocketServiceImpl>(connection_ptr, UserID(-1))) {
  if (!connection_ptr)
    throw std::system_error(qls::qls_errc::null_socket_pointer);
}

SocketService::~SocketService() noexcept = default;

std::shared_ptr<Connection<asio::ip::tcp::socket>>
SocketService::get_connection_ptr() const {
  return m_impl->m_connection_ptr;
}

asio::awaitable<void>
SocketService::process(std::string_view data,
                       std::shared_ptr<qls::DataPackage> pack) {
  auto async_send =
      [this](std::string_view data, long long requestID = 0,
             DataPackage::DataPackageType type = DataPackage::Unknown,
             int sequence = 0,
             int sequenceSize = 1) -> asio::awaitable<std::size_t> {
    auto pack = qls::DataPackage::makePackage(data);
    pack->requestID = requestID;
    pack->sequence = sequence;
    pack->type = type;
    // Send data to the connection
    co_return co_await asio::async_write(
        m_impl->m_connection_ptr->socket, asio::buffer(pack->packageToString()),
        asio::bind_executor(m_impl->m_connection_ptr->strand,
                            asio::use_awaitable));
  };

  // Check whether the user was logged in
  if (m_impl->m_jsonProcess.getLocalUserID() == -1ll &&
      pack->type != DataPackage::Text) {
    co_await async_send(makeErrorMessage("You haven't logged in!").to_string(),
                        pack->requestID, DataPackage::Text);
    co_return;
  }

  // Check the type of the data pack
  switch (pack->type) {
  case DataPackage::Text:
    // json data type
    co_await async_send((co_await m_impl->m_jsonProcess.processJsonMessage(
                             qjson::to_json(data), *this))
                            .to_string(),
                        pack->requestID, DataPackage::Text);
    co_return;
  case DataPackage::FileStream:
    // file stream type
    co_await async_send(makeErrorMessage("Error type").to_string(),
                        pack->requestID,
                        DataPackage::Text); // Temporarily return an error
    co_return;
  case DataPackage::Binary:
    // binary stream type
    co_await async_send(makeErrorMessage("Error type").to_string(),
                        pack->requestID,
                        DataPackage::Text); // Temporarily return an error
    co_return;
  default:
    // unknown type
    co_await async_send(makeErrorMessage("Error type").to_string(),
                        pack->requestID, DataPackage::Text);
    co_return;
  }
  co_return;
}

} // namespace qls
