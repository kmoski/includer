#include "fs_hash.h"
void fs_hash::hash_dir(const fs::path &dir, std::map<std::string, std::vector<std::string>> &hash) {
  for (auto &i : fs::recursive_directory_iterator(dir)) {
    auto &tmp = i.path();
    hash[tmp.filename().string()].push_back(tmp.string());
  }
}
