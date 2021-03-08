#ifndef INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35
#define INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35

# include<filesystem>
#  include   <vector>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using std::vector;
using std::ifstream;
using std::string;

class includer {
  bool continue_parse(const fs::directory_entry &i) noexcept;
  bool find_include(const fs::directory_entry &i, const char *sub) noexcept;
  bool check_include(const fs::directory_entry &i, string inc) noexcept;

  fs::path main_dir;
  vector<fs::path> include_dirs;
  ifstream ifile;
  unsigned long long line;
  string tmp;
  char tmp_c;
 public:
  includer(const int &argc, char *argv[]);
  ~includer() = default;
  void recursive_iteration() noexcept;
};

#endif //INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35
