#ifndef MD_CTX_PROXY_HPP
#define MD_CTX_PROXY_HPP

#include <cstdio>
#include <cstring>
#include <openssl/evp.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "md_proxy.hpp"

namespace qls {

class md_ctx_proxy {
public:
  md_ctx_proxy(md_proxy &md_ctx) : md_proxy_(md_ctx) {
    if (!md_proxy_) {
      throw std::logic_error("md_proxy has been moved");
    }
    digest_context_ = EVP_MD_CTX_new();
    if (digest_context_ == nullptr) {
      throw std::runtime_error("EVP_MD_CTX_new() returned NULL");
    }
    if (EVP_DigestInit(digest_context_, md_proxy_.get_native()) != 1) {
      EVP_MD_CTX_free(digest_context_);
      throw std::runtime_error("EVP_DigestInit() failed");
    }
  }

  md_ctx_proxy(const md_ctx_proxy &) = delete;
  md_ctx_proxy(md_ctx_proxy &&md_ctx) noexcept
      : md_proxy_(md_ctx.md_proxy_),
        digest_context_(std::exchange(md_ctx.digest_context_, nullptr)) {}

  md_ctx_proxy &operator=(const md_ctx_proxy &) = delete;
  md_ctx_proxy &operator=(md_ctx_proxy &&) noexcept = delete;

  ~md_ctx_proxy() noexcept {
    if (digest_context_ != nullptr) {
      EVP_MD_CTX_free(digest_context_);
    }
  }

  EVP_MD_CTX *get_md_ctx_native() noexcept { return digest_context_; }

  operator bool() { return static_cast<bool>(digest_context_); }

  template <class... Args>
    requires requires(Args &&...args) {
      (std::string_view(std::forward<Args>(args)), ...);
    }
  std::string operator()(Args &&...args) {
    if (digest_context_ == nullptr) {
      if (!md_proxy_) {
        throw std::logic_error("md_proxy has been moved");
      }
      digest_context_ = EVP_MD_CTX_new();
      if (digest_context_ == nullptr) {
        throw std::runtime_error("EVP_MD_CTX_new() returned NULL");
      }
      if (EVP_DigestInit(digest_context_, md_proxy_.get_native()) != 1) {
        EVP_MD_CTX_free(digest_context_);
        throw std::runtime_error("EVP_DigestInit() failed");
      }
    }
    std::string digest_value;
    int digest_length = EVP_MD_get_size(md_proxy_.get_native());
    if (digest_length <= 0) {
      throw std::runtime_error("EVP_MD_get_size() returned invalid size");
    }

    auto input = [this](std::string_view data) {
      if (EVP_DigestUpdate(digest_context_, data.data(), data.size()) != 1) {
        throw std::runtime_error("EVP_DigestUpdate() failed");
      }
    };

    (input(std::forward<Args>(args)), ...);

    digest_value.resize(digest_length);
    if (EVP_DigestFinal(digest_context_,
                        reinterpret_cast<unsigned char *>(digest_value.data()),
                        nullptr) != 1) {
      throw std::runtime_error("EVP_DigestFinal() failed");
    }
    std::string buffer;
    std::size_t buffer_size = static_cast<std::size_t>(digest_length) * 2;
    buffer.resize(buffer_size + 1);
    char *buffer_pointer = buffer.data();
    for (std::size_t i = 0, j = 0; j < digest_length; i = i + 2, ++j) {
#ifdef _MSC_VER
      ::sprintf_s(
          buffer_pointer + i, buffer_size + 1 - i, "%02x",
          reinterpret_cast<const unsigned char *>(digest_value.c_str())[j]);
#else
      std::sprintf(
          buffer_pointer + i, "%02x",
          reinterpret_cast<const unsigned char *>(digest_value.c_str())[j]);
#endif
    }
    buffer.resize(buffer_size);
    return buffer;
  }

private:
  md_proxy &md_proxy_;
  EVP_MD_CTX *digest_context_;
};

} // namespace qls

#endif // MD_CTX_PROXY_HPP
