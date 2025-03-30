#include "package.h"

#include <system_error>
#include <cstring>
#include <cstdint>

#include "networkEndianness.hpp"
#include "qls_error.h"

void qls::Package::write(std::string_view data)
{
    m_buffer += data;
}

bool qls::Package::canRead() const
{
    if (m_buffer.size() < sizeof(int))
        return false;

    int length = 0;
    std::memcpy(&length, m_buffer.c_str(), sizeof(int));
    length = qls::swapNetworkEndianness(length);
    if (length > INT32_MAX / 2)
        throw std::system_error(qls_errc::data_too_large);
    else if (length > m_buffer.size())
        return false;
    return true;
}

std::size_t qls::Package::firstMsgLength() const
{
    if (m_buffer.size() < sizeof(int))
        return 0;

    int length = 0;
    std::memcpy(&length, m_buffer.c_str(), sizeof(int));
    length = qls::swapNetworkEndianness(length);
    return std::size_t(length);
}

std::string qls::Package::read()
{
    if (!canRead())
        throw std::system_error(qls_errc::incomplete_package);
    else if (!firstMsgLength())
        throw std::system_error(qls_errc::empty_length);

    std::string result = m_buffer.substr(0, firstMsgLength());
    m_buffer = m_buffer.substr(firstMsgLength());

    return result;
}

std::string_view qls::Package::readBuffer() const
{
    return m_buffer;
}

void qls::Package::setBuffer(std::string_view b)
{
    m_buffer = b;
}

std::string qls::Package::makePackage(std::string_view data)
{
    int length = static_cast<int>(data.size());
    std::string result;
    result.resize(sizeof(int));
    std::memcpy(result.data(), &length, sizeof(int));
    result += data;

    return result;
}
