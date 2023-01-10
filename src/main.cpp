#include <CoreServices/CoreServices.h>

#include <iostream>

#include "include/watcher-manager.hpp"

int main() {
  directory::WatcherManager watcher_manager{};

  std::string my_path = "./test_dir";

  if (watcher_manager.StartWatching(my_path)) {
    std::cout << "Started" << std::endl;
  }

  std::string line;
  while (std::cin >> line) {
    if (line == "q") {
      break;
    }
    if (watcher_manager.StartWatching(line)) {
      std::cout << "Added a new watcher" << std::endl;
    } else {
      std::cout << "Watcher already exists" << std::endl;
    }
  }

  return 0;
}
