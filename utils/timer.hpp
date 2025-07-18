#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <ctime>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace qls {

class Timers {
public:
  Timers()
      : m_isRunning(true), m_thread(std::bind(&Timers::workFunction, this)) {}

  ~Timers() {
    m_isRunning = false;
    m_cv.notify_all();

    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

  // 添加函数任务
  template <typename Func, typename... Args>
  bool addTask(const std::string &taskName, long long interval, Func &&func,
               Args &&...args) {
    if (std::find(m_taskNames.begin(), m_taskNames.end(), taskName) !=
        m_taskNames.end()) {
      return false;
    }

    {
      std::unique_lock lock(m_mutex);
      m_taskNames.push_back(taskName);
      m_tasks.push(
          {std::clock(), taskName,
           std::bind(std::forward<Func>(func), std::forward<Args>(args)...),
           interval});
    }
    m_cv.notify_all();

    return true;
  }

  // 删除任务
  void removeTask(long long postion) {
    if (postion >= static_cast<long long>(m_taskNames.size())) {
      throw std::logic_error("The postion is invalid.");
    }

    std::unique_lock lock(m_mutex);

    std::string taskName = *(m_taskNames.begin() + postion);

    m_removeTask.push(taskName);
    m_taskNames.erase(m_taskNames.begin() + postion);

    m_cv.notify_all();
  }

  // 获取任务列表
  const std::vector<std::string> &getTaskList() const { return m_taskNames; }

protected:
  struct Task {
    std::clock_t clock = 0;
    std::string taskName;
    std::function<void()> function;
    long long interval = 0;

    Task &operator=(const Task &task) {
      if (this == &task) {
        return *this;
      }

      clock = task.clock;
      taskName = task.taskName;
      function = task.function;
      interval = task.interval;
      return *this;
    }
  };

  struct Lesser {
    bool operator()(const Task &task1, const Task &task2) {
      return task1.clock > task2.clock;
    }
  };

  void workFunction() {
    while (m_isRunning) {
      std::unique_lock lock(m_mutex);
      m_cv.wait(lock, [&]() {
        return (!m_tasks.empty() && m_tasks.top().clock <= std::clock()) ||
               !m_isRunning;
      });
      if (!m_isRunning) {
        return;
      }

      if (!m_removeTask.empty() &&
          m_removeTask.front() == m_tasks.top().taskName) {
        m_removeTask.pop();
        m_tasks.pop();
        lock.unlock();
        if (!m_tasks.empty()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(
              (m_tasks.top().clock - std::clock()) > 0
                  ? (m_tasks.top().clock - std::clock())
                  : 0));
        }
        continue;
      }

      Task task = m_tasks.top();
      m_tasks.pop();
      task.function();
      task.clock += static_cast<std::clock_t>(task.interval);
      m_tasks.push(std::move(task));
      lock.unlock();
      std::this_thread::sleep_for(
          std::chrono::milliseconds((m_tasks.top().clock - std::clock()) > 0
                                        ? (m_tasks.top().clock - std::clock())
                                        : 0));
    }
  }

private:
  std::vector<std::string> m_taskNames;
  std::priority_queue<Task, std::vector<Task>, Lesser> m_tasks;
  std::queue<std::string> m_removeTask;

  std::condition_variable m_cv;
  std::mutex m_mutex;
  std::thread m_thread;
  std::atomic<bool> m_isRunning;
};

} // namespace qls

#endif // !TIMER_HPP
