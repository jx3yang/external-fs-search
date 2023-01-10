#ifndef __WATCHER_HPP
#define __WATCHER_HPP

#include <CoreServices/CoreServices.h>

#include <string>

namespace directory {

/**
 * A Watcher monitors for the events that happen inside a directory (including the subdirectories)
 */
class Watcher {
 public:
  Watcher(std::string path_to_directory, CFAbsoluteTime latency);
  ~Watcher();
  Watcher& operator=(Watcher&&);
  Watcher(Watcher&&);

  /**
   * API
   */
  bool StartWatching(dispatch_queue_t dispatch_queue, FSEventStreamCallback callback);
  bool StopWatching();

  /**
   * Deleted initialization methods
   */
 public:
  // no copies
  Watcher(const Watcher&) = delete;
  Watcher& operator=(const Watcher&) = delete;

 private:
  CFStringRef path_ref_;
  CFArrayRef path_array_ref_;
  CFAbsoluteTime latency_;
  FSEventStreamRef stream_;

  bool StopWatching(FSEventStreamRef stream);
};
}  // namespace directory

#endif  // __WATCHER_HPP
