#ifndef OSSL_PROXY_HPP
#define OSSL_PROXY_HPP

#include <openssl/evp.h>
#include <stdexcept>

namespace qls
{
    class ossl_proxy
    {
    public:
        ossl_proxy()
        {
            library_context_ = OSSL_LIB_CTX_new();
            if (library_context_ == nullptr)
                throw std::runtime_error("OSSL_LIB_CTX_new() returned NULL");
        }

        ossl_proxy(const ossl_proxy&) = delete;
        ossl_proxy(ossl_proxy&& o) noexcept
        {
            library_context_ = o.library_context_;
            o.library_context_ = nullptr;
        }

        ossl_proxy& operator=(const ossl_proxy&) = delete;
        ossl_proxy& operator=(ossl_proxy&& o) noexcept
        {
            if (this == &o)
                return *this;
            if (library_context_ != nullptr)
                OSSL_LIB_CTX_free(library_context_);
            library_context_ = o.library_context_;
            o.library_context_ = nullptr;
            return *this;
        }

        ~ossl_proxy() noexcept
        {
            if (library_context_ != nullptr)
                OSSL_LIB_CTX_free(library_context_);
        }

        OSSL_LIB_CTX* get_native()
        {
            return library_context_;
        }

    private:
        OSSL_LIB_CTX* library_context_;
    };
} // namespace qls


#endif // OSSL_PROXY_HPP
