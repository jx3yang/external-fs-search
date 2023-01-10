#ifndef __WATCHER_MANAGER_HPP
#define __WATCHER_MANAGER_HPP

#include <CoreServices/CoreServices.h>

#include <unordered_map>

#include "include/watcher.hpp"

namespace directory {

class WatcherManager {
 public:
  WatcherManager();
  ~WatcherManager();

  /**
   * API
   */
  bool StartWatching(std::string path_to_directory);
  bool StartWatching(std::string path_to_directory, CFAbsoluteTime latency);
  bool StopWatching(std::string path_to_directory);

  /**
   * Deleted initialization methods
   */
 public:
  // no copies, no moves
  WatcherManager(const WatcherManager&) = delete;
  WatcherManager(WatcherManager&&) = delete;
  WatcherManager& operator=(const WatcherManager&) = delete;
  WatcherManager& operator=(WatcherManager&&) = delete;

 private:
  std::unordered_map<std::string, Watcher> watchers_;
  dispatch_queue_t dispatch_queue_;
};

}  // namespace directory
#endif
