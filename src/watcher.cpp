#include <watcher.hpp>
#include <iostream>

namespace directory {

Watcher::Watcher(std::string path_to_directory, CFAbsoluteTime latency)
    : path_ref_{CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)path_to_directory.data(),
                                        path_to_directory.size(), kCFStringEncodingUTF8, false)},
      path_array_ref_{CFArrayCreate(NULL, (const void**)&path_ref_, 1, NULL)},
      latency_{latency},
      stream_{nullptr},
      path_{path_to_directory} {}

Watcher& Watcher::operator=(Watcher&& other) {
  path_ref_ = std::exchange(other.path_ref_, nullptr);
  path_array_ref_ = std::exchange(other.path_array_ref_, nullptr);
  latency_ = other.latency_;
  stream_ = std::exchange(other.stream_, nullptr);
  return *this;
}

Watcher::Watcher(Watcher&& other)
    : path_ref_{std::exchange(other.path_ref_, nullptr)},
      path_array_ref_{std::exchange(other.path_array_ref_, nullptr)},
      latency_{other.latency_},
      stream_{std::exchange(other.stream_, nullptr)} {}

Watcher::~Watcher() {
  StopWatching(stream_);
  if (path_ref_) {
    CFRelease(path_ref_);
  }
  if (path_array_ref_) {
    CFRelease(path_array_ref_);
  }
}

FSEventStreamRef Watcher::StartWatching(dispatch_queue_t dispatch_queue, FSEventStreamCallback callback, bool file_level) {
  FSEventStreamCreateFlags flags = kFSEventStreamCreateFlagWatchRoot;
  if (file_level) {
    flags |= kFSEventStreamCreateFlagFileEvents;
  }
  stream_ = FSEventStreamCreate(NULL, callback, NULL, path_array_ref_, kFSEventStreamEventIdSinceNow, latency_, flags);
  FSEventStreamSetDispatchQueue(stream_, dispatch_queue);
  if (FSEventStreamStart(stream_)) {
    return stream_;
  }
  return NULL;
}

FSEventStreamRef Watcher::StartWatching(dispatch_queue_t dispatch_queue, FSEventStreamCallback callback) {
  return StartWatching(dispatch_queue, callback, true);
}

bool Watcher::StopWatching() { return StopWatching(stream_); }

bool Watcher::StopWatching(FSEventStreamRef stream) {
  if (stream) {
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);
    stream = nullptr;
    return true;
  }
  return false;
}

}  // namespace directory
