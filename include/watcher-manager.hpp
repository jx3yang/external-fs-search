#ifndef __WATCHER_MANAGER_HPP
#define __WATCHER_MANAGER_HPP

#include <CoreServices/CoreServices.h>

#include <unordered_map>
#include <watcher.hpp>

namespace directory {

class WatcherManager {
 public:
  static WatcherManager& get();

 public:
  ~WatcherManager();

  /**
   * API
   */
  bool StartWatching(std::string path_to_directory);
  bool StartWatching(std::string path_to_directory, bool file_level);
  bool StartWatching(std::string path_to_directory, bool file_level, FSEventStreamCallback callback);
  bool StartWatching(std::string path_to_directory, CFAbsoluteTime latency, bool file_level);
  bool StartWatching(std::string path_to_directory, CFAbsoluteTime latency, bool file_level, FSEventStreamCallback callback);
  bool StartWatchingVolumes();
  bool StopWatching(std::string path_to_directory);

  std::string findByStream(ConstFSEventStreamRef stream) {
    auto it = streams_map_.find((FSEventStreamRef) stream);
    if (it != streams_map_.end()) {
      return it->second;
    }
    return "";
  }

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
  WatcherManager();
  std::unordered_map<std::string, Watcher> watchers_;
  std::unordered_map<FSEventStreamRef, std::string> streams_map_;
  dispatch_queue_t dispatch_queue_;
};

}  // namespace directory
#endif
