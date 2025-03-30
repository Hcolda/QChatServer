#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>
#include <fstream>
#include <atomic>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <functional>

namespace Log
{
    
template<typename T>
concept StreamType = requires (T t) {
    std::cout << t;
};

class Logger
{
public:
    enum class LogMode
    {
        LogINFO = 0,
        LogWARNING,
        LogERROR,
        LogCRITICAL,
        LogDEBUG
    };

    /**
     * @brief Default constructor.
     * Opens the log file and starts the logging thread.
     * Throws std::runtime_error if the log file cannot be opened.
     */
    Logger() :
        m_isRunning(true),
        m_thread(std::bind(&Logger::workFunction, this))
    {
        if (!openFile())
            throw std::runtime_error("Could not open the log file.");
    }

    /**
     * @brief Destructor.
     * Stops the logging thread and closes the log file.
     */
    ~Logger()
    {
        m_isRunning = false;
        m_cv.notify_all();

        if (m_thread.joinable())
            m_thread.join();
    }

    /**
     * @brief Logs an informational message.
     * @tparam Args Variadic template arguments.
     * @param args Arguments to be logged.
     */
    template<typename... Args>
        requires ((StreamType<Args>) && ...)
    constexpr void info(Args&&... args)
    {
        print(LogMode::LogINFO, std::forward<Args>(args)...);
    }

    /**
     * @brief Logs a warning message.
     * @tparam Args Variadic template arguments.
     * @param args Arguments to be logged.
     */
    template<typename... Args>
        requires ((StreamType<Args>) && ...)
    constexpr void warning(Args&&... args)
    {
        print(LogMode::LogWARNING, std::forward<Args>(args)...);
    }

    /**
     * @brief Logs an error message.
     * @tparam Args Variadic template arguments.
     * @param args Arguments to be logged.
     */
    template<typename... Args>
        requires ((StreamType<Args>) && ...)
    constexpr void error(Args&&... args)
    {
        print(LogMode::LogERROR, std::forward<Args>(args)...);
    }

    /**
     * @brief Logs a critical error message.
     * @tparam Args Variadic template arguments.
     * @param args Arguments to be logged.
     */
    template<typename... Args>
        requires ((StreamType<Args>) && ...)
    constexpr void critical(Args&&... args)
    {
        print(LogMode::LogCRITICAL, std::forward<Args>(args)...);
    }

    /**
     * @brief Logs a debug message.
     * Only logs if _DEBUG macro is defined.
     * @tparam Args Variadic template arguments.
     * @param args Arguments to be logged.
     */
    template<typename... Args>
        requires ((StreamType<Args>) && ...)
    constexpr void debug(Args&&... args)
    {
#ifdef _DEBUG
        print(LogMode::LogDEBUG, std::forward<Args>(args)...);
#endif // _DEBUG
    }

    /**
     * @brief Prints a log message to both console and file.
     * @tparam Args Variadic template arguments.
     * @param mode Log mode (INFO, WARNING, etc.).
     * @param args Arguments to be logged.
     */
    template<typename... Args>
        requires ((StreamType<Args>) && ...)
    void print(LogMode mode, Args... args)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_msgQueue.push(std::bind([mode = std::move(mode), this](auto&&... args) {
            std::string modeString;

            switch (mode) {
            case Logger::LogMode::LogINFO:
                modeString = "[INFO]";
                break;
            case Logger::LogMode::LogWARNING:
                modeString = "[WRANING]";
                break;
            case Logger::LogMode::LogERROR:
                modeString = "[ERROR]";
                break;
            case Logger::LogMode::LogCRITICAL:
                modeString = "[CRITICAL]";
                break;
            case Logger::LogMode::LogDEBUG:
                modeString = "[DEBUG]";
                break;
            default:
                break;
            }

            std::cout << generateTimeFormatString() << modeString;
            ((std::cout << args), ...);
            std::cout << '\n';

            m_file << generateTimeFormatString() << modeString;
            ((m_file << std::forward<decltype(args)>(args)), ...);
            m_file << std::endl;
            }, std::forward<Args>(args)...));
        m_cv.notify_all();
    }

protected:
    /**
     * @brief Generates a formatted log file name based on current date.
     * @return Formatted log file name.
     */
    static std::string generateFileName()
    {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream ss;
#ifdef _MSC_VER
        std::tm local_tm = {};
        if (::localtime_s(&local_tm, &t))
            throw std::runtime_error("Could not get the ctime tm.");
        ss << std::put_time(&local_tm, "%Y-%m-%d.log");
#else
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d.log");
#endif
        return ss.str();
    }

    /**
     * @brief Generates a formatted timestamp string.
     * @return Formatted timestamp string.
     */
    static std::string generateTimeFormatString()
    {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::stringstream ss;
#ifdef _MSC_VER
        std::tm local_tm = {};
        if (::localtime_s(&local_tm, &t))
            throw std::runtime_error("Could not get the ctime tm.");
        ss << std::put_time(&local_tm, "[%H:%M:%S]");
#else
        ss << std::put_time(std::localtime(&t), "[%H:%M:%S]");
#endif
        return ss.str();
    }

    /**
     * @brief Opens the log file for writing.
     * Creates the log directory if it doesn't exist.
     * @return True if file opened successfully, false otherwise.
     */
    bool openFile()
    {
        std::filesystem::create_directory("./logs");
        m_file.open("./logs/" + generateFileName(), std::ios_base::app);
        return static_cast<bool>(m_file);
    }

    /**
     * @brief Background function for logging thread.
     * Waits for messages in the queue and processes them asynchronously.
     */
    void workFunction()
    {
        while (m_isRunning) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&]() {return !m_msgQueue.empty() || !m_isRunning; });
            if (!m_isRunning && m_msgQueue.empty())
                return;
            else if (m_msgQueue.empty())
                continue;
            std::function<void()> getFunc = std::move(m_msgQueue.front());
            m_msgQueue.pop();
            getFunc();
        }
    }

private:
    std::ofstream                       m_file;         /**< Log file stream */
    std::condition_variable             m_cv;           /**< Condition variable for thread synchronization */
    std::mutex                          m_mutex;        /**< Mutex for protecting shared resources */
    std::atomic<bool>                   m_isRunning;    /**< Atomic flag for controlling thread termination */
    std::queue<std::function<void()>>   m_msgQueue;     /**< Queue for storing log message functions */
    std::thread                         m_thread;       /**< Thread for asynchronous logging */
};

} // namespace qls

#endif // !LOGGER_HPP
