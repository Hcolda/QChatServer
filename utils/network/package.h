#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <string_view>

namespace qls
{
    
/**
 * @brief A class to handle data packages.
 */
class Package final
{
public:
    Package() = default;
    ~Package() noexcept = default;

    Package(const Package&) = delete;
    Package(Package&&) = delete;

    Package& operator=(const Package&) = delete;
    Package& operator=(Package&&) = delete;

    /**
     * @brief Writes data into the class.
     * @param data The binary data to write.
     */
    void write(std::string_view data);

    /**
     * @brief Checks if data can be read from the package.
     * @return true if data can be read, false otherwise.
     */
    [[nodiscard]] bool canRead() const;

    /**
     * @brief Gets the length of the first message in the package.
     * @return The length of the first message.
     */
    [[nodiscard]] std::size_t firstMsgLength() const;

    /**
     * @brief Reads a data package.
     * @return The data package.
     */
    [[nodiscard]] std::string read();

    /**
     * @brief Reads the buffer data in the package.
     * @return The buffer as a string.
     */
    [[nodiscard]] std::string_view readBuffer() const;

    /**
     * @brief Sets the buffer with the given data.
     * @param buffer The data to set in the buffer.
     */
    void setBuffer(std::string_view buffer);

    /**
     * @brief Creates a data package from binary data.
     * @param data The binary data.
     * @return The binary data wrapped in a data package.
     */
    [[nodiscard]] static std::string makePackage(std::string_view data);

private:
    std::string m_buffer; ///< The buffer to store the data.
};

} // namespace qls

#endif // !PACKAGE_H
