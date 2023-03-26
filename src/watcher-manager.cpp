#include <iostream>
#include <filesystem>
#include <sys/stat.h>
#include <watcher-manager.hpp>
#include <directory-structure-manager.hpp>

namespace {
const std::vector<std::string> kIgnoredDirectoris = { "node_modules", "venv", "env" };

const std::string kVolumesPath = "/Volumes";
const std::string kDefaultVolume = kVolumesPath + "/" + "Macintosh HD";

const CFAbsoluteTime kDefaultLatency = 3.0;  // 3 seconds
constexpr int kFileChangedEventsMask = kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemFinderInfoMod |
                                       kFSEventStreamEventFlagItemInodeMetaMod | kFSEventStreamEventFlagItemModified |
                                       kFSEventStreamEventFlagItemRemoved | kFSEventStreamEventFlagMustScanSubDirs;

inline bool flagsHaveValue(FSEventStreamEventFlags flags, int value) {
  return ((flags & value) != 0);
}

#define DEBUG_FLAG(flags, value)        \
  do {                                  \
    if (flagsHaveValue(flags, value)) {  \
      std::cout << #value "\n";         \
    }                                   \
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

#define DEFINE_CALLBACK(callback_name) \
  void callback_name(ConstFSEventStreamRef stream_ref, void *client_callback_info, size_t num_events, void *event_paths, \
                      const FSEventStreamEventFlags event_flags[], const FSEventStreamEventId event_ids[])

DEFINE_CALLBACK(callback_volumes) {
  const char **paths = (const char **)event_paths;
  std::cout << "Callback Volumes called\n";
  for (size_t i = 0; i < num_events; i++) {
    auto flags = event_flags[i];

    if (flagsHaveValue(flags, kFSEventStreamEventFlagMount)) {
      std::cout << "Mounted\n";
    }

    if (flagsHaveValue(flags, kFSEventStreamEventFlagUnmount)) {
      std::cout << "Unmounted\n";
    }
  }
}

DEFINE_CALLBACK(callback) {
  const char **paths = (const char **)event_paths;
  struct stat st{};
  std::cout << "Callback called\n";
  auto path = directory::WatcherManager::get().findByStream(stream_ref);
  if (path.empty()) {
    std::cout << "Could not find path\n";
    return;
  }

  // std::cout << "Stream: " << stream_ref << '\n';
  // std::cout << "Path: " << directory::WatcherManager::get().findByStream(stream_ref) << '\n';
  for (size_t i = 0; i < num_events; i++) {
    // flags are unsigned long, IDs are uint64_t
    std::cout << "Change " << (uint64_t)event_ids[i] << " in " << paths[i] << '\n';
    // print_flags(event_flags[i]);
    if (stat(paths[i], &st) == 0) {
      // std::cout << "Inode: " << st.st_ino << std::endl;
      directory::DirectoryStructureManager::get().Update(path, paths[i], st);
    } else {
      directory::DirectoryStructureManager::get().Remove(path, paths[i]);
      // std::cout << "Could not stat" << std::endl;
    }
    directory::DirectoryStructureManager::get().print();
  }
}

void initial_indexing(const std::string& root_path) {
  struct stat st;
  std::vector<std::string> files;
  auto& directory_structure_manager = directory::DirectoryStructureManager::get();
  // for (const auto& entry : std::filesystem::recursive_directory_iterator(root_path)) {
  //   // std::cout << entry.path() << '\n';
  //   // if (stat(entry.path().c_str(), &st)) {
  //   //   directory_structure_manager.Update(root_path, entry.path(), st);
  //   // }
  // }
  auto is_hidden = [](const std::string& path) {
    return (path[path.find_last_of('/')+1] == '.');
  };

  for (const auto& entry : std::filesystem::directory_iterator(root_path)) {
    if (!is_hidden(entry.path())) {
      files.emplace_back(entry.path());
    }
  }

  while (!files.empty()) {
    auto path = files.back();
    files.pop_back();
    bool is_ignored = false;
    for (const auto& ignored : kIgnoredDirectoris) {
      if (path.find(ignored) != std::string::npos) {
        is_ignored = true;
        break;
      }
    }

    if (is_ignored) {
      continue;
    }

    if (stat(path.c_str(), &st) == 0) {
      directory_structure_manager.Update(root_path, path, st);
      if (S_ISDIR(st.st_mode)) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
          if (!is_hidden(entry.path())) {
            files.emplace_back(entry.path());
          }
        }
      }
    }
  }
}

}  // namespace

namespace directory {

WatcherManager::WatcherManager()
    : watchers_{},
      streams_map_{},
      dispatch_queue_{dispatch_queue_create("WatcherManagerDispatchQueue", nullptr)} {}

WatcherManager& WatcherManager::get() {
  static WatcherManager the_instance{};
  return the_instance;
}

WatcherManager::~WatcherManager() {
  if (dispatch_queue_) {
    dispatch_release(dispatch_queue_);
    dispatch_queue_ = nullptr;
  }
}

bool WatcherManager::StartWatchingVolumes() {
  if (StartWatching(kVolumesPath, false, &callback_volumes)) {
    std::cout << "Watching " << kVolumesPath << " for Volumes\n";

    for (const auto& entry : std::filesystem::directory_iterator(kVolumesPath)) {
      if (entry.path() == kDefaultVolume) {
        continue;
      }
      if (StartWatching(entry.path())) {
        std::cout << "Watching " << entry.path() << " for changes\n";
        initial_indexing(entry.path());
      }
    }
    return true;
  }
  return false;
}

bool WatcherManager::StartWatching(std::string path_to_directory) {
  return StartWatching(path_to_directory, true);
}

bool WatcherManager::StartWatching(std::string path_to_directory, bool file_level) {
  return StartWatching(path_to_directory, file_level, &callback);
}

bool WatcherManager::StartWatching(std::string path_to_directory, bool file_level, FSEventStreamCallback callback) {
  return StartWatching(path_to_directory, kDefaultLatency, file_level, callback);
}

bool WatcherManager::StartWatching(std::string path_to_directory, CFAbsoluteTime latency, bool file_level, FSEventStreamCallback callback) {
  if (watchers_.find(path_to_directory) != watchers_.end()) {
    return false;
  }

  Watcher new_watcher{path_to_directory, latency};
  auto stream = new_watcher.StartWatching(dispatch_queue_, callback, file_level);
  if (stream != nullptr) {
    streams_map_.emplace(stream, path_to_directory);
    watchers_.emplace(path_to_directory, std::move(new_watcher));
    DirectoryStructureManager::get().AddDirectory(path_to_directory);
    return true;
  }

  return false;
}
}  // namespace directory
