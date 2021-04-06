#ifndef INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35
#define INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35

# include<filesystem>
#  include   <vector>
#include <fstream>
#include <string>
#include <map>

namespace fs = std::filesystem;
using std::vector;
using std::ifstream;
using std::string;
using std::map;

enum class msg_type {
  none,
  warning,
  error,
  found,
  mosk,
};

static const map<msg_type, string> msg_str = { // NOLINT(cert-err58-cpp)
    {msg_type::none, "" },
    {msg_type::warning, "warning" },
    {msg_type::error, "error" },
    {msg_type::found, "found" },
    {msg_type::mosk, "mosk" },
};

struct include_t {
  fs::path path;  ///> include path
  ifstream::pos_type position;  ///> position in file
  bool exp;  ///> type of include "" (true) or <> (false)
};

class includer {
  bool continue_parse() noexcept;
  bool parse_comment() noexcept;
  bool parse_block_comment() noexcept;
  bool parse_string() noexcept;
  bool parse_directive() noexcept;
  bool parse_include(bool exp, const ifstream::pos_type &position) noexcept;
  bool parse_end_line(msg_type type = msg_type::none, bool def = true) noexcept;

  void message(const string &info1, const string &info2, msg_type type = msg_type::error) const noexcept;
  bool read_next(const string &info, msg_type type = msg_type::error) noexcept;

  fs::path main_dir;
  const fs::directory_entry *i_fs_file;
  ifstream i_file;
  unsigned long long line;
  char tmp_c;
  bool endl_default = false;
  map<fs::path, vector<include_t>> includes;
 public:
  includer(const int &argc, char *argv[]);
  ~includer() = default;
  void recursive_initialization() noexcept;
};

#endif //INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35
