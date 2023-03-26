#include <directory-structure.hpp>
#include <memory>
#include <sstream>
#include <sys/stat.h>

namespace {

inline bool isDir(const struct stat& st) {
  return S_ISDIR(st.st_mode);
}

inline void update(directory::FileNode& fileNode, const struct stat& st) {
  fileNode.inode.inode_id = st.st_ino;
  fileNode.inode.metadata.file_type = isDir(st) ? directory::FileType::DIR : directory::FileType::FILE;
}

inline std::shared_ptr<directory::FileNode> makeFileNode(std::string file_path, directory::inode_id_t inode_id, directory::time_t last_modified, directory::FileType file_type) {
  return std::make_shared<directory::FileNode>(inode_id, file_path, last_modified, file_type);
}

inline std::shared_ptr<directory::FileNode> makeFileNode(std::string file_path, const struct stat& st) {
  return makeFileNode(file_path, st.st_ino, st.st_mtimespec.tv_sec, isDir(st) ? directory::FileType::DIR : directory::FileType::FILE);
}

// file_path must be of the form /.../.../...
inline std::vector<std::string> splitPath(const std::string& file_path) {
  std::stringstream ss{file_path};
  std::vector<std::string> parts;
  std::string part;
  while (std::getline(ss, part, '/')) {
    if (part.size() > 0) {
      parts.emplace_back(part);
    }
  }
  return parts;
}

inline bool checkValid(const std::string& root_path, const std::string& file_path) {
  size_t found = file_path.find(root_path);
  return found == 0;
}

inline std::pair<std::shared_ptr<directory::FileNode>, std::unordered_map<std::string, std::shared_ptr<directory::FileNode>>::iterator>
findParentNode(std::shared_ptr<directory::FileNode> root_node, std::string file_path) {
  auto parts = splitPath(file_path.substr(root_node->inode.metadata.file_name.size()));
  std::shared_ptr<directory::FileNode> parent = nullptr;
  std::shared_ptr<directory::FileNode> current = root_node;
  std::unordered_map<std::string, std::shared_ptr<directory::FileNode>>::iterator it{};
  for (const auto& part : parts) {
    parent = current;
    it = parent->subdirectories.find(part);
    if (it == parent->subdirectories.end()) {
      return { nullptr, {} };
    }
    current = it->second;
  }
  return { parent, it };
}

inline std::shared_ptr<directory::FileNode> fillFileNodes(std::shared_ptr<directory::FileNode> root_node, std::string file_path, const struct stat& st) {
  if (!checkValid(root_node->inode.metadata.file_name, file_path)) {
    return nullptr;
  }
  auto parts = splitPath(file_path.substr(root_node->inode.metadata.file_name.size()));
  auto current_node = root_node;
  size_t i = 0;
  std::string incremental_file_path = "";
  while (i < parts.size()) {
    if (current_node->subdirectories.find(parts[i]) == current_node->subdirectories.end()) {
      break;
    }
    current_node = current_node->subdirectories[parts[i++]];
    incremental_file_path += "/" + parts[i];
  }

  // not sure if this could happen, but added for completeness
  struct stat incremental_stat;
  while (i < parts.size() - 1) {
    incremental_file_path += "/" + parts[i];
    if (stat(incremental_file_path.c_str(), &incremental_stat) == 0) {
      current_node->subdirectories[parts[i]] = makeFileNode(parts[i], incremental_stat);
      ++i;
      // current_node->subdirectories[parts[i++]] = makeFileNode(incremental_file_path, incremental_stat);
    } else {
      // something went wrong
      return nullptr;
    }
  }

  auto it = current_node->subdirectories.find(parts.back());
  if (it == current_node->subdirectories.end()) {
    // current_node->subdirectories[parts.back()] = makeFileNode(file_path, st);
    current_node->subdirectories[parts.back()] = makeFileNode(parts.back(), st);
  } else {
    update(*(it->second), st);
  }
  return current_node->subdirectories[parts.back()];
}

} // namespace

namespace directory {

DirectoryStructure::DirectoryStructure(std::string root_directory, const struct stat& st)
  : root_node_{makeFileNode(root_directory, st)},
    file_node_cache_{},
    file_name_index_{}
    {}

void DirectoryStructure::Update(std::string file_path, const struct stat& st) {
  auto node = fillFileNodes(root_node_, file_path, st);
  // if (node == nullptr) {
  //   return;
  // }
  // auto it = file_node_cache_.cache.find(file_path);
  // if (it == file_node_cache_.cache.end()) {
  //   file_node_cache_.deque.emplace_front(node);
  //   file_node_cache_.cache[file_path] = file_node_cache_.deque.begin();
  //   // TODO: if over size, evict
  // } else {
  //   file_node_cache_.deque.erase(it->second);
  //   file_node_cache_.deque.emplace_front(node);
  //   file_node_cache_.cache[file_path] = file_node_cache_.deque.begin();
  // }
}

void DirectoryStructure::remove(std::shared_ptr<FileNode> file_node) {
  auto iter = file_node_cache_.cache.find(file_node->inode.metadata.file_name);
  if (iter != file_node_cache_.cache.end()) {
    file_node_cache_.deque.erase(iter->second);
    file_node_cache_.cache.erase(iter);
  }
  for (auto &n : file_node->subdirectories) {
    remove(n.second);
  }
}

void DirectoryStructure::Remove(std::string file_path) {
  auto [parent, it] = findParentNode(root_node_, file_path);
  if (parent == nullptr) {
    return;
  }
  auto current_node = it->second;
  parent->subdirectories.erase(it);
  remove(current_node);
}

} // namespace directory
