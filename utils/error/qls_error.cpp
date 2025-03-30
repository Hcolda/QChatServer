#include "qls_error.h"

class qls_error_category: public std::error_category
{
    virtual const char* name() const noexcept override;
    virtual std::string message(int Errval) const override;
};

const char *qls_error_category::name() const noexcept
{
    return "qls error";
}

std::string qls_error_category::message(int Errval) const
{
    using qls::qls_errc;
    switch (static_cast<qls::qls_errc>(Errval))
    {
    // system error
    case qls_errc::null_pointer:
        return "pointer is null";
    // package error
    case qls_errc::incomplete_package:
        return "packge is incomplete";
    case qls_errc::empty_length:
        return "data is empty";
    case qls_errc::invalid_data:
        return "data is invalid";
    case qls_errc::data_too_small:
        return "data is too small";
    case qls_errc::data_too_large:
        return "data is too large";
    case qls_errc::hash_mismatched:
        return "hash mismatched";

    // network error
    case qls_errc::null_tls_context:
        return "tls context is null";
    case qls_errc::null_tls_callback_handle:
        return "tls callback_handle is null";
    case qls_errc::connection_test_failed:
        return "connection test failed";
    case qls_errc::null_socket_pointer:
        return "pointer of socket is null";
    case qls_errc::socket_pointer_existed:
        return "socket pointer already exists";
    case qls_errc::socket_pointer_not_existed:
        return "socket pointer doesn't exist";

    // user error
    case qls_errc::password_already_set:
        return "user has set password";
    case qls_errc::password_mismatched:
        return "password mismatched";
    case qls_errc::user_existed:
        return "user already exists";
    case qls_errc::user_not_existed:
        return "user doesn't exist";

    // verification error
    case qls_errc::verification_existed:
        return "verification already exists";
    case qls_errc::verification_not_existed:
        return "verification doesn't exist";
    case qls_errc::invalid_verification:
        return "invalid verification";

    // private room error
    case qls_errc::private_room_existed:
        return "private room already exists";
    case qls_errc::private_room_not_existed:
        return "private room doesn't exist";
    case qls_errc::private_room_unable_to_use:
        return "room can't be used";

    // group room error
    case qls_errc::group_room_existed:
        return "group room already exists";
    case qls_errc::group_room_not_existed:
        return "group room doesn't exist";
    case qls_errc::group_room_unable_to_use:
        return "room can't be used";
    case qls_errc::group_room_user_level_invalid:
        return "level of user(s) in group room is invalid";

    // permission error
    case qls_errc::no_permission:
        return "no permission";
    case qls_errc::permission_denied:
        return "permission denied";
        
    default:
        break;
    }

    return "unknown error";
}

static qls_error_category QLSErrCategory;

std::error_code qls::make_error_code(qls::qls_errc errc) noexcept
{
    return std::error_code(static_cast<int>(errc), QLSErrCategory);
}
