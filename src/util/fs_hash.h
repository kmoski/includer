#ifndef FS_HASH_H_122EAB41888E4E9DB67B3E5088D2D4C1
#define FS_HASH_H_122EAB41888E4E9DB67B3E5088D2D4C1

#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

class fs_hash {
  static void hash_dir(const fs::path &dir, std::map<std::string, std::vector<std::string>> &hash);
};

#endif //FS_HASH_H_122EAB41888E4E9DB67B3E5088D2D4C1
