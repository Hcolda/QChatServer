#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <utility>

namespace Log {

template <typename T>
concept StreamType = requires(T data) { std::cout << data; };

class Logger {
public:
  enum class LogMode : std::uint8_t {
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
  Logger()
      : m_isRunning(true), m_thread(std::bind(&Logger::workFunction, this)) {
    if (!openFile()) {
      throw std::runtime_error("Could not open the log file.");
    }
  }

  /**
   * @brief Destructor.
   * Stops the logging thread and closes the log file.
   */
  ~Logger() { join(); }

  /**
   * @brief Logs an informational message.
   * @tparam Args Variadic template arguments.
   * @param args Arguments to be logged.
   */
  template <typename... Args>
    requires((StreamType<Args>) && ...)
  constexpr void info(Args &&...args) {
    print(LogMode::LogINFO, std::forward<Args>(args)...);
  }

  /**
   * @brief Logs a warning message.
   * @tparam Args Variadic template arguments.
   * @param args Arguments to be logged.
   */
  template <typename... Args>
    requires((StreamType<Args>) && ...)
  constexpr void warning(Args &&...args) {
    print(LogMode::LogWARNING, std::forward<Args>(args)...);
  }

  /**
   * @brief Logs an error message.
   * @tparam Args Variadic template arguments.
   * @param args Arguments to be logged.
   */
  template <typename... Args>
    requires((StreamType<Args>) && ...)
  constexpr void error(Args &&...args) {
    print(LogMode::LogERROR, std::forward<Args>(args)...);
  }

  /**
   * @brief Logs a critical error message.
   * @tparam Args Variadic template arguments.
   * @param args Arguments to be logged.
   */
  template <typename... Args>
    requires((StreamType<Args>) && ...)
  constexpr void critical(Args &&...args) {
    print(LogMode::LogCRITICAL, std::forward<Args>(args)...);
  }

  /**
   * @brief Logs a debug message.
   * Only logs if _DEBUG macro is defined.
   * @tparam Args Variadic template arguments.
   * @param args Arguments to be logged.
   */
  template <typename... Args>
    requires((StreamType<Args>) && ...)
  constexpr void debug(Args &&...args) {
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
  template <typename... Args>
    requires((StreamType<Args>) && ...)
  void print(LogMode mode, Args... args) {
    std::pmr::polymorphic_allocator<> allocator{&this->m_memory_resouce};
    auto ptr = allocator.new_object<PrintTask<Args...>>(
        this, mode, std::make_tuple(std::move(args)...));
    std::unique_ptr<PrintTask<Args...>, PrintTaskDeleter> task_ptr(
        ptr, PrintTaskDeleter{this, sizeof(PrintTask<Args...>)});

    std::unique_lock lock(m_mutex);
    m_msgQueue.emplace(std::move(task_ptr));
    lock.unlock();
    m_cv.notify_all();
  }

  void join() {
    m_isRunning = false;
    m_cv.notify_all();
    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

protected:
  struct BasePrintTask {
    virtual void operator()() = 0;
    virtual ~BasePrintTask() noexcept = default;
  };

  template <class... Args> struct PrintTask : public BasePrintTask {
    Logger *this_logger;
    LogMode mode;
    std::tuple<Args...> tuple;

    PrintTask(Logger *logger, LogMode mode, std::tuple<Args...> time_point)
        : this_logger(logger), mode(mode), tuple(std::move(time_point)) {}

    virtual ~PrintTask() noexcept = default;

    template <typename Tuple, size_t... Is>
    static void printTupleImpl(const Tuple &time_point,
                               std::index_sequence<Is...> index) {
      ((std::cout << std::get<Is>(time_point)), ...);
    }

    template <typename Tuple, size_t... Is>
    void writeTupleImpl(const Tuple &time_point,
                        std::index_sequence<Is...> index) {
      ((this_logger->m_file << std::get<Is>(time_point)), ...);
    }

    virtual void operator()() override {
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
      printTupleImpl(tuple, std::index_sequence_for<Args...>{});
      std::cout << '\n';

      this_logger->m_file << generateTimeFormatString() << modeString;
      writeTupleImpl(tuple, std::index_sequence_for<Args...>{});
      this_logger->m_file << '\n';
      this_logger->m_file.flush();
    }
  };

  struct PrintTaskDeleter {
    Logger *this_logger;
    std::size_t size;

    void operator()(BasePrintTask *ptr) const {
      ptr->~BasePrintTask();
      this_logger->m_memory_resouce.deallocate(ptr, size);
    }
  };

  /**
   * @brief Generates a formatted log file name based on current date.
   * @return Formatted log file name.
   */
  static std::string generateFileName() {
    auto time_point =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream string_stream;
#ifdef _MSC_VER
    std::tm local_tm = {};
    if (::localtime_s(&local_tm, &time_point)) {
      throw std::runtime_error("Could not get the ctime tm.");
    }
    string_stream << std::put_time(&local_tm, "%Y-%mode-%d.log");
#else
    string_stream << std::put_time(std::localtime(&time_point),
                                   "%Y-%mode-%d.log");
#endif
    return string_stream.str();
  }

  /**
   * @brief Generates a formatted timestamp string.
   * @return Formatted timestamp string.
   */
  static std::string generateTimeFormatString() {
    auto time_point =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream string_stream;
#ifdef _MSC_VER
    std::tm local_tm = {};
    if (::localtime_s(&local_tm, &time_point)) {
      throw std::runtime_error("Could not get the ctime tm.");
    }
    string_stream << std::put_time(&local_tm, "[%H:%M:%S]");
#else
    string_stream << std::put_time(std::localtime(&time_point), "[%H:%M:%S]");
#endif
    return string_stream.str();
  }

  /**
   * @brief Opens the log file for writing.
   * Creates the log directory if it doesn'time_point exist.
   * @return True if file opened successfully, false otherwise.
   */
  bool openFile() {
    std::filesystem::create_directory("./logs");
    m_file.open("./logs/" + generateFileName(), std::ios_base::app);
    return static_cast<bool>(m_file);
  }

  /**
   * @brief Background function for logging thread.
   * Waits for messages in the queue and processes them asynchronously.
   */
  void workFunction() {
    while (true) {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [&]() { return !m_msgQueue.empty() || !m_isRunning; });
      if (!m_isRunning && m_msgQueue.empty()) {
        return;
      }
      if (m_msgQueue.empty()) {
        continue;
      }
      auto task = std::move(m_msgQueue.front());
      m_msgQueue.pop();
      std::invoke(*task);
    }
  }

private:
  std::ofstream m_file;
  std::condition_variable m_cv;
  std::mutex m_mutex;
  std::atomic<bool> m_isRunning;
  std::queue<std::unique_ptr<BasePrintTask, PrintTaskDeleter>> m_msgQueue;
  std::thread m_thread;
  std::pmr::synchronized_pool_resource m_memory_resouce;
};

} // namespace Log

#endif // !LOGGER_HPP
