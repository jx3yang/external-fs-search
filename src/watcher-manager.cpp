#include "include/watcher-manager.hpp"

#include <iostream>

namespace {
const CFAbsoluteTime kDefaultLatency = 3.0;  // 3 seconds
constexpr int kFileChangedEventsMask = kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemFinderInfoMod |
                                       kFSEventStreamEventFlagItemInodeMetaMod | kFSEventStreamEventFlagItemModified |
                                       kFSEventStreamEventFlagItemRemoved | kFSEventStreamEventFlagMustScanSubDirs;

#define DEBUG_FLAG(flags, value) \
  do {                           \
    if ((flags & value) != 0) {  \
      std::cout << #value "\n";  \
    }                            \
  } while (0)

void print_flags(FSEventStreamEventFlags flags) {
  if (flags == kFSEventStreamEventFlagNone) {
    std::cout << "kFSEventStreamEventFlagNone\n";
  }
  DEBUG_FLAG(flags, kFSEventStreamEventFlagMustScanSubDirs);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagUserDropped);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagKernelDropped);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagEventIdsWrapped);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagHistoryDone);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagRootChanged);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagMount);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagUnmount);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemChangeOwner);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemCreated);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemFinderInfoMod);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemInodeMetaMod);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemIsDir);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemIsFile);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemIsHardlink);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemIsLastHardlink);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemIsSymlink);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemModified);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemRemoved);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemRenamed);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagItemXattrMod);
  DEBUG_FLAG(flags, kFSEventStreamEventFlagOwnEvent);
}

void callback(ConstFSEventStreamRef stream_ref, void *client_callback_info, size_t num_events, void *event_paths,
              const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId event_ids[]) {
  const char **paths = (const char **)event_paths;
  std::cout << "Callback called\n";
  for (size_t i = 0; i < num_events; i++) {
    // flags are unsigned long, IDs are uint64_t
    std::cout << "Change " << (uint64_t)event_ids[i] << " in " << paths[i] << '\n';
    print_flags(event_flags[i]);
  }
}
}  // namespace

namespace directory {

WatcherManager::WatcherManager()
    : watchers_{}, dispatch_queue_{dispatch_queue_create("WatcherManagerDispatchQueue", nullptr)} {}

WatcherManager::~WatcherManager() {
  if (dispatch_queue_) {
    dispatch_release(dispatch_queue_);
    dispatch_queue_ = nullptr;
  }
}

bool WatcherManager::StartWatching(std::string path_to_directory) {
  return StartWatching(path_to_directory, kDefaultLatency);
}

bool WatcherManager::StartWatching(std::string path_to_directory, CFAbsoluteTime latency) {
  if (watchers_.find(path_to_directory) != watchers_.end()) {
    // TODO: return better type than simply bool
    // already has a watcher
    return false;
  }

  Watcher new_watcher{path_to_directory, latency};
  if (new_watcher.StartWatching(dispatch_queue_, &callback)) {
    watchers_.emplace(path_to_directory, std::move(new_watcher));
    return true;
  }

  return false;
}
}  // namespace directory
