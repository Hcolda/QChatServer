#include "dataPackage.h"

#include <cstring>
#include <memory_resource>
#include <string>
#include <system_error>

#include "networkEndianness.hpp"
#include "qls_error.h"

namespace qls {

static std::pmr::synchronized_pool_resource local_datapack_sync_pool;

std::shared_ptr<DataPackage> DataPackage::makePackage(std::string_view data,
                                                      DataPackageType type,
                                                      LengthType sequenceSize,
                                                      LengthType sequence,
                                                      RequestIDType requestID) {
  const std::size_t lenth = sizeof(DataPackage) + data.size();
  void *mem = local_datapack_sync_pool.allocate(static_cast<LengthType>(lenth));
  std::memset(mem, 0, lenth);
  std::shared_ptr<DataPackage> package(
      static_cast<DataPackage *>(mem), [lenth](DataPackage *data_package) {
        data_package->~DataPackage();
        local_datapack_sync_pool.deallocate(data_package, lenth);
      });
  package->length = static_cast<LengthType>(lenth);
  std::memcpy(package->data, data.data(), data.size());
  package->type = type;
  package->sequenceSize = sequenceSize;
  package->sequence = sequence;
  package->requestID = requestID;
  return package;
}

std::shared_ptr<DataPackage>
DataPackage::stringToPackage(std::string_view data) {
  // Check if the package data is too small
  if (data.size() < sizeof(DataPackage)) {
    throw std::system_error(qls_errc::data_too_small);
  }

  // Data package length
  LengthType size = 0;
  std::memcpy(&size, data.data(), sizeof(LengthType));
  if (!isBigEndianness()) {
    size = swapEndianness(size);
  }

  // Error handling if data package length does not match actual size,
  // if length is smaller than the default package size
  if (size != data.size() || size < sizeof(DataPackage)) {
    throw std::system_error(qls_errc::invalid_data);
  }

  // Allocate memory and construct the DataPackage
  void *mem = local_datapack_sync_pool.allocate(size);
  std::memset(mem, 0, size);
  std::shared_ptr<DataPackage> package(
      static_cast<DataPackage *>(mem),
      [lenth = size](DataPackage *data_package) {
        data_package->~DataPackage();
        local_datapack_sync_pool.deallocate(data_package, lenth);
      });
  // Copy the data from string
  std::memcpy(package.get(), data.data(), size);

  // Process data in package
  if (!isBigEndianness()) {
    // Endianness conversion
    package->length = swapEndianness(package->length);
    package->type = static_cast<DataPackageType>(
        swapEndianness(static_cast<LengthType>(package->type)));
    package->sequenceSize = swapEndianness(package->sequenceSize);
    package->sequence = swapEndianness(package->sequence);
    package->requestID = swapEndianness(package->requestID);
  }

  return package;
}

std::string DataPackage::packageToString() const {
  using namespace qls;
  std::string strdata;
  strdata.resize(this->length);
  // Copy this memory data into strdata
  std::memcpy(strdata.data(), this, this->length);
  // Converse the string pointer to DataPackage pointor to process data
  DataPackage *package = reinterpret_cast<DataPackage *>(strdata.data());

  // Process string data
  if (!isBigEndianness()) {
    // Endianness conversion
    package->length = swapEndianness(package->length);
    package->type = static_cast<DataPackageType>(
        swapEndianness(static_cast<LengthType>(package->type)));
    package->sequenceSize = swapEndianness(package->sequenceSize);
    package->sequence = swapEndianness(package->sequence);
    package->requestID = swapEndianness(package->requestID);
  }

  return strdata;
}

std::size_t DataPackage::getPackageSize() const noexcept {
  return static_cast<std::size_t>(this->length);
}

std::size_t DataPackage::getDataSize() const noexcept {
  return static_cast<std::size_t>(this->length) - sizeof(DataPackage);
}

std::string DataPackage::getData() const {
  return {reinterpret_cast<const char *>(this->data), this->getDataSize()};
}

void DataPackage::getData(std::string &buffer) const {
  buffer.assign(reinterpret_cast<const char *>(this->data),
                this->getDataSize());
}

void DataPackage::getData(std::pmr::string &buffer) const {
  buffer.assign(reinterpret_cast<const char *>(this->data),
                this->getDataSize());
}

} // namespace qls
