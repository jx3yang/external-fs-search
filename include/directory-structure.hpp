#ifndef __DIRECTORY_STRUCTURE_HPP
#define __DIRECTORY_STRUCTURE_HPP

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/stat.h>

#include <iostream>

namespace directory {

using inode_id_t = int;
using time_t = unsigned long;

enum FileType {
  FILE,
  DIR,
};

struct FileMetadata {
  std::string file_name;
  time_t last_modified;
  FileType file_type;
};

struct FileInode {
  inode_id_t inode_id;
  FileMetadata metadata;
};

struct FileNode {
  FileNode(inode_id_t inode_id, std::string file_name, time_t last_modified, FileType file_type)
    : inode{inode_id, { file_name, last_modified, file_type }},
      subdirectories{}
      {}

  size_t count() const noexcept {
    size_t c = 1;
    for (const auto& e : subdirectories) {
      c += e.second->count();
    }
    return c;
  }

  // ~FileNode() {
  //   std::cout << "~FileNode()" << std::endl;
  // }

  FileInode inode;
  // file_name -> file_node
  std::unordered_map<std::string, std::shared_ptr<FileNode>> subdirectories;
};

struct FileNodeCache {
  std::unordered_map<std::string, std::list<std::shared_ptr<FileNode>>::iterator> cache;
  std::list<std::shared_ptr<FileNode>> deque;
};

class DirectoryStructure {
 public:
  using FileNameIndex = std::unordered_map<std::string, std::vector<std::shared_ptr<FileNode>>>;
  using FilterFunc = const std::function<bool(const std::string&)>&;

 public:
  DirectoryStructure(std::string root_directory, const struct stat& st);

  /**
   * Updates the entry at `file_path` with information from `st`.
   * If the entry does not exist, a new entry is created.
  */
  void Update(std::string file_path, const struct stat& st);

  void Remove(std::string file_path);

  /**
   * Returns all the files that match the filter
  */
  FileNameIndex GetFiles(FilterFunc filter);

  void print(const FileNode& node) const noexcept {
    std::cout << "Ino: " << node.inode.inode_id << " Path: " << node.inode.metadata.file_name
              << " Type: " << (node.inode.metadata.file_type == FileType::DIR ? "DIR" : "FILE")
              << " Subs: ";
    for (const auto& e : node.subdirectories) {
      std::cout << e.first << " ";
    }
    std::cout << '\n';
    for (const auto& e : node.subdirectories) {
      print(*(e.second));
    }
  }

  void print() const noexcept {
    print(*root_node_);
  }

  size_t count() const noexcept {
    return root_node_->count();
  }

 private:
  std::shared_ptr<FileNode> root_node_;
  FileNodeCache file_node_cache_;
  FileNameIndex file_name_index_;

  void remove(std::shared_ptr<FileNode> file_node);
};

}  // namespace directory
#endif
