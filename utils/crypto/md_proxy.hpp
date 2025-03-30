#ifndef MD_PROXY_HPP
#define MD_PROXY_HPP

#include <cstring>
#include <cstdio>
#include <openssl/evp.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "ossl_proxy.hpp"

namespace qls
{
    class md_proxy
    {
    public:
        md_proxy(ossl_proxy& o, std::string_view algorithm):
            ossl_proxy_(o)
        {
            message_digest_ = EVP_MD_fetch(ossl_proxy_.get_native(),
                                            algorithm.data(), nullptr);
            if (message_digest_ == nullptr)
                throw std::runtime_error("EVP_MD_fetch() returned NULL");
            digest_context_ = EVP_MD_CTX_new();
            if (digest_context_ == nullptr) {
                EVP_MD_free(message_digest_);
                throw std::runtime_error("EVP_MD_CTX_new() returned NULL");
            }
            if (EVP_DigestInit(digest_context_, message_digest_) != 1) {
                EVP_MD_free(message_digest_);
                EVP_MD_CTX_free(digest_context_);
                throw std::runtime_error("EVP_DigestInit() failed");
            }
        }

        md_proxy(const md_proxy&) = delete;
        md_proxy(md_proxy&& md) noexcept:
            ossl_proxy_(md.ossl_proxy_),
            message_digest_(nullptr),
            digest_context_(nullptr)
        {
            std::swap(message_digest_, md.message_digest_);
            std::swap(digest_context_, md.digest_context_);
        }

        md_proxy& operator=(const md_proxy&) = delete;
        md_proxy& operator=(md_proxy&& md) noexcept
        {
            if (this == &md)
                return *this;

            if (message_digest_ != nullptr)
                EVP_MD_free(message_digest_);
            if (digest_context_ != nullptr)
                EVP_MD_CTX_free(digest_context_);
            message_digest_ = md.message_digest_;
            digest_context_ = md.digest_context_;
            md.digest_context_ = nullptr;
            md.message_digest_ = nullptr;
            return *this;
        }

        ~md_proxy() noexcept
        {
            if (message_digest_ != nullptr)
                EVP_MD_free(message_digest_);
            if (digest_context_ != nullptr)
                EVP_MD_CTX_free(digest_context_);
        }

        EVP_MD* get_md_native() noexcept
        {
            return message_digest_;
        }

        EVP_MD_CTX* get_md_ctx_native() noexcept
        {
            return digest_context_;
        }

        operator bool()
        {
            return message_digest_ && digest_context_;
        }

        template<class... Args>
            requires requires (Args&&... args) { (std::string_view(std::forward<Args>(args)), ...); }
        std::string operator()(Args&&... args)
        {
            std::string digest_value;
            int digest_length = EVP_MD_get_size(message_digest_);
            if (digest_length <= 0)
                throw std::runtime_error("EVP_MD_get_size() returned invalid size");

            auto input = [this](std::string_view data) {
                if (EVP_DigestUpdate(digest_context_, data.data(), data.size()) != 1)
                    throw std::runtime_error("EVP_DigestUpdate() failed");
            };

            (input(std::forward<Args>(args)), ...);

            digest_value.resize(digest_length);
            if (EVP_DigestFinal(digest_context_,
                                reinterpret_cast<unsigned char*>(digest_value.data()),
                                nullptr) != 1) {
                throw std::runtime_error("EVP_DigestFinal() failed");
            }
            std::string buffer;
            std::size_t buffer_size = digest_length * 2;
            buffer.resize(buffer_size + 1);
            char* buffer_pointer = buffer.data();
            for (std::size_t i = 0, j = 0; j < digest_length; i = i + 2, ++j) {
#ifdef _MSC_VER
                ::sprintf_s(buffer_pointer + i,
                            buffer_size + 1 - i,
                            "%02x",
                            reinterpret_cast<const unsigned char*>(digest_value.c_str())[j]);
#else
                std::sprintf(buffer_pointer + i,
                             "%02x",
                             reinterpret_cast<const unsigned char*>(digest_value.c_str())[j]);
#endif
            }
            buffer.resize(buffer_size);
            return buffer;
        }

    private:
        ossl_proxy& ossl_proxy_;
        EVP_MD*     message_digest_;
        EVP_MD_CTX* digest_context_;
    };
} // namespace qls

#endif // MD_PROXY_HPP
