#ifndef NETWORK_ENDIANNESS_HPP
#define NETWORK_ENDIANNESS_HPP

#include <bit>
#include <concepts>
#include <cstddef>

namespace qls {

/// @brief Determine if the system is big endianness
/// @return True if it is big endianness
[[deprecated("Outdated")]] consteval bool isBigEndianness();

/// @brief Convert number of endianness
/// @tparam T Type of integral
/// @param value Integral of the other endianness
/// @return Integral of new endianness
template <typename T>
  requires std::integral<T>
[[nodiscard]] constexpr T swapEndianness(T value) {
  T result = 0;
  for (std::size_t i = 0; i < sizeof(value); ++i) {
    result = (result << 8) | ((value >> (8 * i)) & 0xFF);
  }
  return result;
}

/// @brief Convert if the network endianness is different from local system
/// @tparam T Type of integral
/// @param value Integral of the other endianness
/// @return Integral of new endianness
template <typename T>
  requires std::integral<T>
[[nodiscard]] inline T swapNetworkEndianness(T value) {
  if constexpr (std::endian::native == std::endian::little) {
    return swapEndianness(value);
  }
  return value;
}

} // namespace qls

#endif // !NETWORK_ENDIANNESS_HPP
