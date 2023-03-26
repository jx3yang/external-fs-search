#ifndef __DIRECTORY_STRUCTURE_MANAGER_HPP
#define __DIRECTORY_STRUCTURE_MANAGER_HPP

#include <directory-structure.hpp>
#include <string>
#include <unordered_map>
#include <sys/stat.h>

namespace directory {

class DirectoryStructureManager {
 public:
  bool AddDirectory(std::string path);
  static DirectoryStructureManager& get();

  void print() const noexcept {
    for (const auto& e : directory_structures_) {
      e.second.print();
    }
  }

  void Update(std::string root_dir, std::string path, const struct stat& st) {
    auto it = directory_structures_.find(root_dir);
    if (it == directory_structures_.end()) {
      return;
    }
    it->second.Update(path, st);
  }

  void Remove(std::string root_dir, std::string path) {
        auto it = directory_structures_.find(root_dir);
    if (it == directory_structures_.end()) {
      return;
    }
    it->second.Remove(path);
  }

  size_t count() const noexcept {
    size_t c = 0;
    for (const auto& e : directory_structures_) {
      c += e.second.count();
    }
    return c;
  }

 private:
  DirectoryStructureManager();
  std::unordered_map<std::string, DirectoryStructure> directory_structures_;
  // DirectoryStructureManager* singleton = nullptr;
};

} // namespace directory

#endif
