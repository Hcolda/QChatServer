#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H
namespace qls {

/**
 * @class DataManager
 * @brief Manages user data and interactions with the database.
 */
class DataManager final {
public:
  DataManager() = default;
  DataManager(const DataManager &) = delete;
  DataManager(DataManager &&) = delete;
  ~DataManager() = default;

  DataManager &operator=(const DataManager &) = delete;
  DataManager &operator=(DataManager &&) = delete;

  /**
   * @brief Initializes the data manager.
   */
  void init();
};

} // namespace qls

#endif // !DATA_MANAGER_H
