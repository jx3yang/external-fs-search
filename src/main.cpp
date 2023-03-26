#include <CoreServices/CoreServices.h>

#include <iostream>

#include <watcher-manager.hpp>
#include <directory-structure-manager.hpp>
#include <filesystem>

int main() {
  directory::WatcherManager& watcher_manager = directory::WatcherManager::get();

  std::string my_path = std::filesystem::canonical("./test_dir");

  if (watcher_manager.StartWatching(my_path)) {
    std::cout << "Started on path " << my_path << std::endl;
  }

  watcher_manager.StartWatchingVolumes();

  std::cout << "Ready" << std::endl;

  // if (watcher_manager.StartWatchingVolumes()) {
  //   std::cout << "Started watching Volumes\n";
  // }

  // std::cout << "Volumes:\n";
  // for (const auto& entry : std::filesystem::directory_iterator("/Volumes")) {
  //   std::cout << entry.path() << std::endl;
  // }

  std::string line;
  while (std::cin >> line) {
    if (line == "q") {
      break;
    }
    if (line == "p") {
      directory::DirectoryStructureManager::get().print();
      continue;
    }
    if (line == "c") {
      std::cout << "Count " << directory::DirectoryStructureManager::get().count() << '\n';
      continue;
    }
    // if (watcher_manager.StartWatching(line)) {
    //   std::cout << "Added a new watcher" << std::endl;
    // } else {
    //   std::cout << "Watcher already exists" << std::endl;
    // }
  }

  return 0;
}
