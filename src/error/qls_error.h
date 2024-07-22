#ifndef QLS_ERROR_H
#define QLS_ERROR_H

#include <string>
#include <system_error>

namespace qls
{
    enum class qls_errc
    {
        OK = 0,

        // package error
        incomplete_package,
        empty_length,
        invalid_data,
        data_too_small,
        data_too_large,
        hash_mismatched,
        
        // network error
        null_tls_context,
        null_tls_callback_handle,
        null_socket_pointer,
        connection_test_failed,
        socket_pointer_existed,
        socket_pointer_not_existed,

        // user error
        password_already_set,
        password_mismatched,
        user_not_existed,

        // private room error
        private_room_not_existed,
        private_room_unable_to_use,

        // group room error
        group_room_not_existed,
        group_room_unable_to_use,

        // permission error
        no_permission,
    };

    std::error_code make_error_code(qls::qls_errc errc) noexcept;
}

namespace std
{
    template <>
    struct is_error_code_enum<qls::qls_errc> : true_type {};
}

#endif // !QLS_ERROR_H