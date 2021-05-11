#ifndef C_INCLUDE_PARSER_H_AE3F33A08E2C4050B67EC42572EE7698
#define C_INCLUDE_PARSER_H_AE3F33A08E2C4050B67EC42572EE7698

#include <filesystem>
#include <fstream>
#include <map>
#include <vector>
#include "../defines.h"

namespace fs = std::filesystem;

struct include_t {
  fs::path path;                     ///> include path
  std::fstream::pos_type position;  ///> position in file
  unsigned long long line;           ///> line in file
  std::fstream::pos_type length;         ///> length of include string
  bool exp;                          ///> type of include "" (true) or <> (false)
  bool overwrite = false;            ///> overwrite this include
};

enum class msg_type {
  none,
  warning,
  error,
  // custom,
};

static const std::map<msg_type, std::string> msg_str = { // NOLINT(cert-err58-cpp)
    {msg_type::none, "" },
    {msg_type::warning, "warning" },
    {msg_type::error, "error" },
    // {msg_type::custom, "custom" },
};

class c_include_parser {
  bool continue_parse() RELEASE_noexcept;
  bool parse_comment() RELEASE_noexcept;
  bool parse_block_comment() RELEASE_noexcept;
  bool parse_string() RELEASE_noexcept;
  bool parse_directive() RELEASE_noexcept;
  bool parse_symbol() RELEASE_noexcept;

  bool parse_d_include() RELEASE_noexcept;

  bool parse_include(bool exp, const std::ifstream::pos_type &position) RELEASE_noexcept;
  bool parse_end_line(bool def = false) RELEASE_noexcept;
  bool skip_space(const std::string &msg) RELEASE_noexcept;

  inline void peek() noexcept;
  bool read_next(const std::string &msg, msg_type type = msg_type::error) RELEASE_noexcept;
  void message_wrong_syntax(const std::string &msg,
               msg_type type = msg_type::warning) RELEASE_noexcept;
  void message(const std::string &msg_info,
               const std::string &msg,
               msg_type type = msg_type::error) RELEASE_noexcept;

  std::ifstream file;
  unsigned long long line{};
  char tmp_c{};

  const fs::path *path{};
  std::vector<include_t> *includes{};
 public:
  bool from_file(const fs::path *path,
                 std::vector<include_t> *includes) RELEASE_noexcept;
  // unsigned long custom_c = 0;
  bool success = true;
};

#endif //C_INCLUDE_PARSER_H_AE3F33A08E2C4050B67EC42572EE7698
