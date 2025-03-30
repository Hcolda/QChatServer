#include "dataPackage.h"

#include <stdexcept>
#include <format>
#include <cstring>
#include <memory_resource>
#include <functional>

#include "networkEndianness.hpp"
#include "qls_error.h"

namespace qls
{

static std::pmr::synchronized_pool_resource local_datapack_sync_pool;

std::shared_ptr<DataPackage> DataPackage::makePackage(std::string_view data)
{
    const int lenth = static_cast<int>(sizeof(DataPackage) + data.size());
    void* mem = local_datapack_sync_pool.allocate(lenth);
    std::memset(mem, 0, lenth);
    std::shared_ptr<DataPackage> package(static_cast<DataPackage*>(mem),
        [lenth](DataPackage* dp) {
            local_datapack_sync_pool.deallocate(dp, static_cast<std::size_t>(lenth));
        });
    package->length = lenth;
    std::memcpy(package->data, data.data(), data.size());
    return package;
}

std::shared_ptr<DataPackage> DataPackage::stringToPackage(std::string_view data)
{
    // Check if the package data is too small
    if (data.size() < sizeof(DataPackage))
        throw std::system_error(qls_errc::data_too_small);

    // Data package length
    int size = 0;
    std::memcpy(&size, data.data(), sizeof(int));
    if (!isBigEndianness())
        size = swapEndianness(size);

    // Error handling if data package length does not match actual size,
    // if length is smaller than the default package size
    if (size != data.size() || size < sizeof(DataPackage))
        throw std::system_error(qls_errc::invalid_data);
    else if (size > INT32_MAX / 2)
        throw std::system_error(qls_errc::data_too_large);

    // Allocate memory and construct the DataPackage
    void* mem = local_datapack_sync_pool.allocate(size);
    std::memset(mem, 0, size);
    std::shared_ptr<DataPackage> package(static_cast<DataPackage*>(mem),
        [lenth = size](DataPackage* dp) {
            local_datapack_sync_pool.deallocate(dp, static_cast<std::size_t>(lenth));
        });
    // Copy the data from string
    std::memcpy(package.get(), data.data(), size);

    // Process data in package
    if (!isBigEndianness()) {
        // Endianness conversion
        package->length = swapEndianness(package->length);
        package->type = static_cast<DataPackageType>(swapEndianness(static_cast<int>(package->type)));
        package->sequenceSize = swapEndianness(package->sequenceSize);
        package->sequence = swapEndianness(package->sequence);
        package->requestID = swapEndianness(package->requestID);

        char* data = reinterpret_cast<char*>(package.get());
        for (std::size_t i = sizeof(DataPackage); i < size - sizeof(DataPackage); ++i) {
            data[i] = swapEndianness(data[i]);
        }
    }

    return package;
}

std::string DataPackage::packageToString() noexcept
{
    using namespace qls;
    std::string strdata;
    strdata.resize(this->length);
    // Copy this memory data into strdata
    std::memcpy(strdata.data(), this, this->length);
    // Converse the string pointer to DataPackage pointor to process data
    DataPackage* package = reinterpret_cast<DataPackage*>(strdata.data());

    // Process string data
    if (!isBigEndianness()) {
        // Endianness conversion
        package->length = swapEndianness(package->length);
        package->type = static_cast<DataPackageType>(swapEndianness(static_cast<int>(package->type)));
        package->sequenceSize = swapEndianness(package->sequenceSize);
        package->sequence = swapEndianness(package->sequence);
        package->requestID = swapEndianness(package->requestID);

        char* data = strdata.data();
        for (std::size_t i = sizeof(DataPackage); i < this->length - sizeof(DataPackage); ++i) {
            data[i] = swapEndianness(data[i]);
        }
    }

    return strdata;
}

std::size_t DataPackage::getPackageSize() noexcept
{
    int size = 0;
    std::memcpy(&size, &(this->length), sizeof(int));
    return std::size_t(size);
}

std::size_t DataPackage::getDataSize() noexcept
{
    int size = 0;
    std::memcpy(&size, &(this->length), sizeof(int));
    return std::size_t(size) - sizeof(DataPackage);
}

std::string DataPackage::getData()
{
    std::string data;
    std::size_t size = this->getDataSize();
    data.resize(size);
    std::memcpy(data.data(), this->data, size);
    return data;
}

} // namespace qls
