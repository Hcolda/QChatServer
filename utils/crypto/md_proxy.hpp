#ifndef MD_PROXY_HPP
#define MD_PROXY_HPP

#include "ossl_proxy.hpp"
#include <openssl/evp.h>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace qls {

class md_proxy {
public:
  md_proxy(ossl_proxy &ossl_proxy, std::string_view algorithm) {
    if (!ossl_proxy)
      throw std::logic_error("ossl_proxy has been moved");
    message_digest_ =
        EVP_MD_fetch(ossl_proxy.get_native(), algorithm.data(), nullptr);
    if (!message_digest_)
      throw std::runtime_error("EVP_MD_fetch() returned NULL");
  }

  md_proxy(ossl_proxy &ossl_proxy, std::string_view algorithm,
           std::string_view properties) {
    if (!ossl_proxy)
      throw std::logic_error("ossl_proxy has been moved");
    message_digest_ = EVP_MD_fetch(ossl_proxy.get_native(), algorithm.data(),
                                   properties.data());
    if (!message_digest_)
      throw std::runtime_error("EVP_MD_fetch() returned NULL");
  }

  md_proxy(const md_proxy &) = delete;
  md_proxy(md_proxy &&md) noexcept
      : message_digest_(std::exchange(md.message_digest_, nullptr)) {}

  ~md_proxy() noexcept {
    if (message_digest_)
      EVP_MD_free(message_digest_);
  }

  md_proxy &operator=(const md_proxy &) = delete;
  md_proxy &operator=(md_proxy &&md) noexcept {
    if (this == &md)
      return *this;

    if (message_digest_)
      EVP_MD_free(message_digest_);
    message_digest_ = std::exchange(md.message_digest_, nullptr);
    return *this;
  }

  EVP_MD *get_native() noexcept { return message_digest_; }

  operator bool() { return message_digest_; }

private:
  EVP_MD *message_digest_;
};

} // namespace qls

#endif // MD_PROXY_HPP
