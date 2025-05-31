#ifndef DATA_PACKAGE_H
#define DATA_PACKAGE_H

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

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
              RequestIDType requestID = 0);

  /**
   * @brief Loads a data package from binary data.
   * @param data Binary data representing a data package.
   * @return Shared pointer to the loaded data package.
   */
  [[nodiscard]] static std::shared_ptr<DataPackage>
  stringToPackage(std::string_view data);

  /**
   * @brief Converts this data package to a binary string.
   * @return Binary data representing this data package.
   */
  [[nodiscard]] std::string packageToString() const;

  /**
   * @brief Gets the size of this data package.
   * @return Size of this data package.
   */
  [[nodiscard]] std::size_t getPackageSize() const noexcept;

  /**
   * @brief Gets the size of the original data in this data package.
   * @return Size of the original data in this data package.
   */
  [[nodiscard]] std::size_t getDataSize() const noexcept;

  /**
   * @brief Gets the original data in this data package.
   * @return Original data in this data package.
   */
  [[nodiscard]] std::string getData() const;

  /**
   * @brief Retrieves the original data by populating the provided buffer.
   * @param[out] buffer The target string to store the data.
   *                    - Existing contents will be cleared and overwritten.
   *                    - For optimal performance with large data, pre-allocate
   * capacity via `buffer.reserve()` to avoid reallocations.
   * @throw std::bad_alloc If memory allocation fails during buffer resize.
   */
  void getData(std::string &buffer) const;

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
  void getData(std::pmr::string &buffer) const;
};

} // namespace qls

#endif // !DATA_PACKAGE_H
