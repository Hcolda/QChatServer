#include "network.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <Json.h>
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/ssl.hpp>
#include <logger.hpp>

#include <dataPackage.hpp>
#include <package.hpp>

namespace qls {
using asio::awaitable;
using asio::ip::tcp;
namespace this_coro = asio::this_coro;
using namespace asio::experimental::awaitable_operators;
using namespace std::placeholders;
using asio::ip::tcp;
using namespace asio;
using namespace std::chrono;

using ssl_socket = asio::ssl::stream<tcp::socket>;

template <class T> static std::string socket2ip(const T &s) {
  auto ep = s.remote_endpoint();
  return std::format("{}:{}", ep.address().to_string(), int(ep.port()));
}

std::string showBinaryData(const std::string &data) {
  auto isShowableCharactor = [](unsigned char ch) -> bool {
    return 32 <= ch && ch <= 126;
  };

  std::string result;

  for (const auto &i : data) {
    if (isShowableCharactor(static_cast<unsigned char>(i)))
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

      // result += "\\x" + (hex.size() == 1 ? "0" + hex : hex);
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

struct NetworkImpl {
  std::string host = "127.0.0.1";
  int port = 55555;

  std::thread work_thread;
  asio::io_context io_context;
  asio::ssl::context ssl_context;
  std::shared_ptr<ssl_socket> socket_ptr;
  std::atomic<bool> is_running;
  std::atomic<bool> is_receiving;
  std::atomic<bool> has_stopped;
  std::mutex mutex;
  std::condition_variable condition_variable;
  Package<DataPackage::LengthType> package;
  asio::ip::tcp::resolver::results_type endpoints;
  std::string input_buffer;

  std::unordered_map<std::string, ReceiveStdStringFunction>
      revceiveStdStringFunction_map;
  std::shared_mutex revceiveStdStringFunction_map_mutex;
  std::unordered_map<std::string, std::function<void()>>
      connectedCallbackFunction_map;
  std::shared_mutex connectedCallbackFunction_map_mutex;
  std::unordered_map<std::string, std::function<void()>>
      disconnectedCallbackFunction_map;
  std::shared_mutex disconnectedCallbackFunction_map_mutex;
  std::unordered_map<std::string, std::function<void(std::error_code)>>
      connectedErrorCallbackFunction_map;
  std::shared_mutex connectedErrorCallbackFunction_map_mutex;

  std::mt19937_64 requestID_mt;
  std::mutex requestID_mt_mutex;
  std::unordered_set<long long> requestID_set;
  std::shared_mutex requestID_set_mutex;
  std::unordered_map<long long,
                     std::function<void(std::shared_ptr<DataPackage>)>>
      requestID2Function_map;
  std::shared_mutex requestID2Function_map_mutex;

  NetworkImpl()
      : ssl_context(asio::ssl::context::tlsv13_client),
        requestID_mt(std::random_device{}()) {
    input_buffer.resize(8192);

    // ssl 配置
    ssl_context.set_default_verify_paths();

    // 设置ssl参数
    ssl_context.set_options(
        asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |
        asio::ssl::context::no_sslv3 | asio::ssl::context::no_tlsv1 |
        asio::ssl::context::no_tlsv1_1 | asio::ssl::context::no_tlsv1_2 |
        asio::ssl::context::single_dh_use);

    // 设置是否验证cert
    ssl_context.set_verify_mode(asio::ssl::verify_none);
    ssl_context.set_verify_callback(
        std::bind(&NetworkImpl::verify_certificate, this, _1, _2));
    // ssl_context.set_verify_mode(asio::ssl::verify_peer);
    // ssl_context.set_verify_callback(ssl::rfc2818_verification("host.name"));

    socket_ptr = std::make_shared<ssl_socket>(io_context, ssl_context);
  }

  ~NetworkImpl() = default;

  bool verify_certificate(bool preverified, asio::ssl::verify_context &ctx) {
    char subject_name[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }
};

Network::Network() : m_network_impl(std::make_unique<NetworkImpl>()) {
  m_network_impl->is_running = true;
  m_network_impl->is_receiving = false;
  m_network_impl->has_stopped = false;

  m_network_impl->work_thread = std::thread([&]() {
    while (m_network_impl->is_running) {
      std::unique_lock lock(m_network_impl->mutex);
      m_network_impl->condition_variable.wait(lock, [&]() {
        return !m_network_impl->is_running ||
               !m_network_impl->endpoints.empty();
      });
      if (!m_network_impl->is_running)
        return;
      if (m_network_impl->endpoints.empty())
        continue;
      asio::co_spawn(m_network_impl->io_context, start_connect(),
                     asio::detached);
      m_network_impl->io_context.run();
    }
  });
}

Network::~Network() noexcept {
  if (m_network_impl->has_stopped)
    return;
  m_network_impl->has_stopped = true;
  m_network_impl->is_running = false;
  if (m_network_impl->is_receiving) {
    m_network_impl->is_receiving = false;
    std::error_code ignored_error;
    ignored_error = m_network_impl->socket_ptr->shutdown(ignored_error);
  }
  m_network_impl->io_context.stop();
  m_network_impl->condition_variable.notify_all();
  if (m_network_impl->work_thread.joinable())
    m_network_impl->work_thread.join();
}

void Network::connect() {
  asio::ip::tcp::resolver resolver(m_network_impl->io_context);
  m_network_impl->is_receiving = false;
  std::unique_lock lock(m_network_impl->mutex);
  m_network_impl->endpoints = resolver.resolve(
      m_network_impl->host, std::to_string(m_network_impl->port));
  m_network_impl->condition_variable.notify_all();
}

void Network::disconnect() {
  m_network_impl->is_receiving = false;
  {
    std::error_code ignored_error;
    ignored_error = m_network_impl->socket_ptr->shutdown(ignored_error);
  }
  m_network_impl->condition_variable.notify_all();
}

void Network::stop() {
  if (m_network_impl->has_stopped)
    return;
  m_network_impl->has_stopped = true;
  m_network_impl->is_running = false;
  if (m_network_impl->is_receiving) {
    m_network_impl->is_receiving = false;
    std::error_code ignored_error;
    ignored_error = m_network_impl->socket_ptr->shutdown(ignored_error);
  }
  m_network_impl->io_context.stop();
  m_network_impl->condition_variable.notify_all();
}

void Network::send_data(const std::string &data) {
  if (!m_network_impl->is_receiving)
    throw std::runtime_error("Socket is not able to use");
  auto wrapper = std::make_unique<StringWrapper>(data);
  StringWrapper *wrapper_pointer = wrapper.get();
  asio::async_write(*m_network_impl->socket_ptr,
                    asio::buffer(wrapper_pointer->data),
                    [wrapper = std::move(wrapper)](auto, auto) {});
}

std::future<std::shared_ptr<DataPackage>>
Network::send_data_with_result_n_option(
    const std::string &data,
    const std::function<void(std::shared_ptr<DataPackage> &)>
        &option_function) {
  if (!m_network_impl->is_receiving)
    throw std::runtime_error("Socket is not able to use");
  std::shared_ptr<std::promise<std::shared_ptr<DataPackage>>> future_result =
      std::make_shared<std::promise<std::shared_ptr<DataPackage>>>();
  send_data_with_option(data, option_function,
                        [future_result](std::shared_ptr<DataPackage> pack) {
                          future_result->set_value(std::move(pack));
                        });
  return future_result->get_future();
}

long long Network::send_data_with_option(
    const std::string &origin_data,
    const std::function<void(std::shared_ptr<DataPackage> &)> &option_function,
    const std::function<void(std::shared_ptr<DataPackage>)>
        &callback_function) {
  if (!m_network_impl->is_receiving)
    throw std::runtime_error("Socket is not able to use");
  if (!option_function || !callback_function)
    throw std::runtime_error("Functions is null");
  auto pack = DataPackage::makePackage(origin_data);
  option_function(pack);
  long long requestId = 0;
  {
    std::unique_lock mt_lock(m_network_impl->requestID_mt_mutex,
                             std::defer_lock);
    std::unique_lock set_lock(m_network_impl->requestID_set_mutex,
                              std::defer_lock),
        map_lock(m_network_impl->requestID2Function_map_mutex, std::defer_lock);
    std::lock(mt_lock, set_lock, map_lock);
    do {
      requestId = m_network_impl->requestID_mt();
    } while (m_network_impl->requestID_set.find(requestId) !=
             m_network_impl->requestID_set.cend());
    m_network_impl->requestID_set.insert(requestId);
  }
  pack->requestID = requestId;
  m_network_impl->requestID2Function_map[requestId] = callback_function;
  send_data(pack->packageToString());
  return requestId;
}

bool Network::add_received_stdstring_callback(const std::string &name,
                                              ReceiveStdStringFunction func) {
  std::unique_lock lock(m_network_impl->revceiveStdStringFunction_map_mutex);
  auto iter = m_network_impl->revceiveStdStringFunction_map.find(name);
  if (iter != m_network_impl->revceiveStdStringFunction_map.end())
    return false;
  m_network_impl->revceiveStdStringFunction_map[name] = std::move(func);
  return true;
}

bool Network::remove_received_stdstring_callback(const std::string &name) {
  std::unique_lock lock(m_network_impl->revceiveStdStringFunction_map_mutex);
  auto iter = m_network_impl->revceiveStdStringFunction_map.find(name);
  if (iter == m_network_impl->revceiveStdStringFunction_map.end())
    return false;
  m_network_impl->revceiveStdStringFunction_map.erase(iter);
  return true;
}

bool Network::add_connected_callback(const std::string &name,
                                     std::function<void()> func) {
  std::unique_lock lock(m_network_impl->connectedCallbackFunction_map_mutex);
  auto iter = m_network_impl->connectedCallbackFunction_map.find(name);
  if (iter != m_network_impl->connectedCallbackFunction_map.end())
    return false;
  m_network_impl->connectedCallbackFunction_map[name] = std::move(func);
  return true;
}

bool Network::remove_connected_callback(const std::string &name) {
  std::unique_lock lock(m_network_impl->connectedCallbackFunction_map_mutex);
  auto iter = m_network_impl->connectedCallbackFunction_map.find(name);
  if (iter == m_network_impl->connectedCallbackFunction_map.end())
    return false;
  m_network_impl->connectedCallbackFunction_map.erase(iter);
  return true;
}

bool Network::add_disconnected_callback(const std::string &name,
                                        std::function<void()> func) {
  std::unique_lock lock(m_network_impl->disconnectedCallbackFunction_map_mutex);
  auto iter = m_network_impl->disconnectedCallbackFunction_map.find(name);
  if (iter != m_network_impl->disconnectedCallbackFunction_map.end())
    return false;
  m_network_impl->disconnectedCallbackFunction_map[name] = std::move(func);
  return true;
}

bool Network::remove_disconnected_callback(const std::string &name) {
  std::unique_lock lock(m_network_impl->disconnectedCallbackFunction_map_mutex);
  auto iter = m_network_impl->disconnectedCallbackFunction_map.find(name);
  if (iter == m_network_impl->disconnectedCallbackFunction_map.end())
    return false;
  m_network_impl->disconnectedCallbackFunction_map.erase(iter);
  return true;
}

bool Network::add_connected_error_callback(
    const std::string &name, std::function<void(std::error_code)> func) {
  std::unique_lock lock(
      m_network_impl->connectedErrorCallbackFunction_map_mutex);
  auto iter = m_network_impl->connectedErrorCallbackFunction_map.find(name);
  if (iter != m_network_impl->connectedErrorCallbackFunction_map.end())
    return false;
  m_network_impl->connectedErrorCallbackFunction_map[name] = std::move(func);
  return true;
}

bool Network::remove_connected_error_callback(const std::string &name) {
  std::unique_lock lock(
      m_network_impl->connectedErrorCallbackFunction_map_mutex);
  auto iter = m_network_impl->connectedErrorCallbackFunction_map.find(name);
  if (iter == m_network_impl->connectedErrorCallbackFunction_map.end())
    return false;
  m_network_impl->connectedErrorCallbackFunction_map.erase(iter);
  return true;
}

void Network::call_connected() {
  std::shared_lock lock(m_network_impl->connectedCallbackFunction_map_mutex);
  for (const auto &[_, func] : m_network_impl->connectedCallbackFunction_map) {
    func();
  }
}

void Network::call_disconnect() {
  std::shared_lock lock(m_network_impl->disconnectedCallbackFunction_map_mutex);
  for (const auto &[_, func] :
       m_network_impl->disconnectedCallbackFunction_map) {
    func();
  }
}

void Network::call_connected_error(const std::error_code &error) {
  std::shared_lock lock(
      m_network_impl->connectedErrorCallbackFunction_map_mutex);
  for (const auto &[_, func] :
       m_network_impl->connectedErrorCallbackFunction_map) {
    func(error);
  }
}

void Network::call_received_stdstring(std::string data) {
  try {
    auto pack = DataPackage::stringToPackage(data);
    long long requestID = pack->requestID;
    if (pack->requestID != 0) {
      std::unique_lock map_lock(m_network_impl->requestID2Function_map_mutex,
                                std::defer_lock),
          set_lock(m_network_impl->requestID_set_mutex, std::defer_lock);
      bool need_moving_data = false;
      std::lock(map_lock, set_lock);
      {
        auto iter = m_network_impl->requestID_set.find(requestID);
        if (iter != m_network_impl->requestID_set.cend()) {
          need_moving_data = true;
          m_network_impl->requestID_set.erase(iter);
        }
      }
      if (need_moving_data) {
        auto iter = m_network_impl->requestID2Function_map.find(requestID);
        if (iter != m_network_impl->requestID2Function_map.cend())
          iter->second(std::move(pack));
        m_network_impl->requestID2Function_map.erase(iter);
        return;
      }
    }
  } catch (...) {
    return;
  }
  std::shared_lock lock(m_network_impl->revceiveStdStringFunction_map_mutex);
  for (const auto &[_, func] : m_network_impl->revceiveStdStringFunction_map) {
    func(data);
  }
}

asio::awaitable<void> Network::start_connect() {
  auto executor = co_await asio::this_coro::executor;

  // timeout function
  auto timeout = [](const std::chrono::steady_clock::duration &duration)
      -> awaitable<void> {
    steady_timer timer(co_await this_coro::executor);
    timer.expires_after(duration);
    co_await timer.async_wait(asio::use_awaitable);
    throw std::system_error(make_error_code(std::errc::timed_out));
  };

  for (std::size_t i = 0; i < 3 && m_network_impl->is_running; ++i) {
    try {
      co_await (asio::async_connect(m_network_impl->socket_ptr->lowest_layer(),
                                    m_network_impl->endpoints.begin(),
                                    m_network_impl->endpoints.end(),
                                    asio::use_awaitable) ||
                timeout(60s));
      co_await (m_network_impl->socket_ptr->async_handshake(
                    asio::ssl::stream_base::client, asio::use_awaitable) ||
                timeout(10s));
      m_network_impl->is_receiving = true;
      call_connected();
      asio::co_spawn(executor, heart_beat_write(), asio::detached);
      for (; m_network_impl->is_running && m_network_impl->is_receiving;) {
        m_network_impl->input_buffer.resize(8192);
        std::size_t size = std::get<0>(
            co_await (m_network_impl->socket_ptr->async_read_some(
                          asio::buffer(m_network_impl->input_buffer),
                          asio::use_awaitable) ||
                      timeout(30s)));
        m_network_impl->package.write(
            {m_network_impl->input_buffer.begin(),
             m_network_impl->input_buffer.begin() + size});
        if (m_network_impl->package.canRead())
          call_received_stdstring(
              std::string(std::move(m_network_impl->package.read())));
      }
      co_return;
    } catch (const std::system_error &e) {
      m_network_impl->is_receiving = false;
      call_connected_error(e.code());
      std::error_code ec;
      ec = m_network_impl->socket_ptr->shutdown(ec);
    } catch (const std::exception &e) {
      m_network_impl->is_receiving = false;
      call_connected_error();
      std::error_code ec;
      ec = m_network_impl->socket_ptr->shutdown(ec);
    }
    asio::steady_timer timer(executor);
    timer.expires_after(10s);
    co_await timer.async_wait(asio::use_awaitable);
  }
  co_return;
}

asio::awaitable<void> Network::heart_beat_write() {
  if (!m_network_impl->is_running || !m_network_impl->is_receiving)
    co_return;
  auto executor = co_await asio::this_coro::executor;
  auto pack = DataPackage::makePackage("heartbeat");
  pack->type = DataPackage::HeartBeat;
  std::string strpack = pack->packageToString();
  asio::steady_timer timer(executor);
  while (m_network_impl->is_running && m_network_impl->is_receiving) {
    timer.expires_after(10s);
    co_await timer.async_wait(asio::use_awaitable);
    try {
      co_await asio::async_write(*m_network_impl->socket_ptr,
                                 asio::buffer(strpack), asio::use_awaitable);
    } catch (...) {
      co_return;
    }
  }
  co_return;
}
} // namespace qls
