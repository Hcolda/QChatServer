#ifndef MD_PROXY_HPP
#define MD_PROXY_HPP

#include "ossl_proxy.hpp"
#include <openssl/evp.h>
#include <stdexcept>
#include <string>
#include <utility>

namespace qls {

class md_proxy {
public:
  md_proxy(ossl_proxy &ossl_proxy, const std::string &algorithm) {
    if (!ossl_proxy) {
      throw std::logic_error("ossl_proxy has been moved");
    }
    message_digest_ =
        EVP_MD_fetch(ossl_proxy.get_native(), algorithm.c_str(), nullptr);
    if (message_digest_ == nullptr) {
      throw std::runtime_error("EVP_MD_fetch() returned NULL");
    }
  }

  md_proxy(ossl_proxy &ossl_proxy, const std::string &algorithm,
           const std::string &properties) {
    if (!ossl_proxy) {
      throw std::logic_error("ossl_proxy has been moved");
    }
    message_digest_ = EVP_MD_fetch(ossl_proxy.get_native(), algorithm.c_str(),
                                   properties.c_str());
    if (message_digest_ == nullptr) {
      throw std::runtime_error("EVP_MD_fetch() returned NULL");
    }
  }

  md_proxy(const md_proxy &) = delete;
  md_proxy(md_proxy &&mdv) noexcept
      : message_digest_(std::exchange(mdv.message_digest_, nullptr)) {}

  ~md_proxy() noexcept {
    if (message_digest_ != nullptr) {
      EVP_MD_free(message_digest_);
    }
  }

  md_proxy &operator=(const md_proxy &) = delete;
  md_proxy &operator=(md_proxy &&mdv) noexcept {
    if (this == &mdv) {
      return *this;
    }

    if (message_digest_ != nullptr) {
      EVP_MD_free(message_digest_);
    }
    message_digest_ = std::exchange(mdv.message_digest_, nullptr);
    return *this;
  }

  EVP_MD *get_native() noexcept { return message_digest_; }

  operator bool() { return static_cast<bool>(message_digest_); }

private:
  EVP_MD *message_digest_;
};

} // namespace qls

#endif // MD_PROXY_HPP
