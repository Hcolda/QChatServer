#ifndef DATA_PACKAGE_H
#define DATA_PACKAGE_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <memory_resource>
#include <string>
#include <string_view>
#include <system_error>

#include "networkEndianness.hpp"
#include "qls_error.h"

namespace qls {

/**
 * @class DataPackage
 * @brief Represents a data package with metadata and binary data.
 */
class DataPackage final {
public:
  using LengthType = std::uint32_t;
  using RequestIDType = long long;
  using DataType = unsigned char;

  enum DataPackageType : LengthType {
    Unknown = 0,
    Text = 1,
    Binary = 2,
    FileStream = 3,
    HeartBeat = 4
  };

private:
#pragma pack(1)
  LengthType length = 0; ///< Length of the data package.
public:
  DataPackageType type = Unknown; ///< Type identifier of the data package.
  LengthType sequenceSize = 1;    ///< Sequence size.
  LengthType sequence = 0;        ///< Sequence number of the data package.
  RequestIDType requestID = 0; ///< Request ID associated with the data package.
private:
  DataType data[0]; ///< Data in the pack
#pragma pack()

public:
  DataPackage() = delete;
  ~DataPackage() noexcept = default;
  DataPackage(const DataPackage &) = delete;
  DataPackage(DataPackage &&) = delete;

  DataPackage &operator=(const DataPackage &) = delete;
  DataPackage &operator=(DataPackage &&) = delete;

  /**
   * @brief Creates a data package from the given data.
   * @return Shared pointer to the created data package.
   */
  [[nodiscard]] static std::shared_ptr<DataPackage>
  makePackage(std::string_view data,
              DataPackageType type = DataPackageType::Unknown,
              LengthType sequenceSize = 1, LengthType sequence = 0,
              RequestIDType requestID = 0) {
    const std::size_t lenth = sizeof(DataPackage) + data.size();
    void *mem =
        local_datapack_sync_pool.allocate(static_cast<LengthType>(lenth));
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

  /**
   * @brief Loads a data package from binary data.
   * @param data Binary data representing a data package.
   * @return Shared pointer to the loaded data package.
   */
  [[nodiscard]] static std::shared_ptr<DataPackage>
  stringToPackage(std::string_view data) {
    // Check if the package data is too small
    if (data.size() < sizeof(DataPackage)) {
      throw std::system_error(qls_errc::data_too_small);
    }

    // Data package length
    LengthType size = 0;
    std::memcpy(&size, data.data(), sizeof(LengthType));
    if (std::endian::native == std::endian::little) {
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
    if constexpr (std::endian::native == std::endian::little) {
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

  /**
   * @brief Converts this data package to a binary string.
   * @return Binary data representing this data package.
   */
  [[nodiscard]] std::string packageToString() const {
    using namespace qls;
    std::string strdata;
    strdata.resize(this->length);
    // Copy this memory data into strdata
    std::memcpy(strdata.data(), this, this->length);
    // Converse the string pointer to DataPackage pointor to process data
    DataPackage *package = reinterpret_cast<DataPackage *>(strdata.data());

    // Process string data
    if constexpr (std::endian::native == std::endian::little) {
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

  /**
   * @brief Gets the size of this data package.
   * @return Size of this data package.
   */
  [[nodiscard]] std::size_t getPackageSize() const noexcept {
    return static_cast<std::size_t>(this->length);
  }

  /**
   * @brief Gets the size of the original data in this data package.
   * @return Size of the original data in this data package.
   */
  [[nodiscard]] std::size_t getDataSize() const noexcept {
    return static_cast<std::size_t>(this->length) - sizeof(DataPackage);
  }

  /**
   * @brief Gets the original data in this data package.
   * @return Original data in this data package.
   */
  [[nodiscard]] std::string getData() const {
    return {reinterpret_cast<const char *>(this->data), this->getDataSize()};
  }

  /**
   * @brief Retrieves the original data by populating the provided buffer.
   * @param[out] buffer The target string to store the data.
   *                    - Existing contents will be cleared and overwritten.
   *                    - For optimal performance with large data, pre-allocate
   * capacity via `buffer.reserve()` to avoid reallocations.
   * @throw std::bad_alloc If memory allocation fails during buffer resize.
   */
  void getData(std::string &buffer) const {
    buffer.assign(reinterpret_cast<const char *>(this->data),
                  this->getDataSize());
  }

  /**
   * @brief Retrieves the original data by populating the provided PMR-enabled
   * buffer.
   * @param[out] buffer The target `std::pmr::string` to store the data.
   *                    - Existing contents will be **cleared and overwritten**.
   *                    - Uses the buffer's **associated memory resource**
   * (e.g., memory pool or arena).
   *                    - For optimal performance, pre-allocate capacity via
   * `buffer.reserve()`.
   * @note This overload requires C++17 or later and is intended for advanced
   * memory management scenarios. Ensure the buffer's allocator matches the
   * desired memory strategy (e.g., monotonic or pooled).
   * @throw std::bad_alloc If the buffer's allocator fails to allocate memory.
   */
  void getData(std::pmr::string &buffer) const {
    buffer.assign(reinterpret_cast<const char *>(this->data),
                  this->getDataSize());
  }

private:
  inline static std::pmr::synchronized_pool_resource local_datapack_sync_pool =
      {};
};

} // namespace qls

#endif // !DATA_PACKAGE_H
