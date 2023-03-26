#include <directory-structure-manager.hpp>
#include <sys/stat.h>

namespace directory {

DirectoryStructureManager::DirectoryStructureManager()
  : directory_structures_{} {}

DirectoryStructureManager& DirectoryStructureManager::get() {
  static DirectoryStructureManager the_instance{};
  return the_instance;
}

bool DirectoryStructureManager::AddDirectory(std::string path) {
  if (directory_structures_.find(path) == directory_structures_.end()) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
      directory_structures_.emplace(path, DirectoryStructure{path, st});
      return true;
    }
  }
  return false;
}

} // namespace directory
