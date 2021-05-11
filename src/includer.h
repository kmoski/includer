#ifndef INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35
#define INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35

#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include "defines.h"
#include "settings.h"
#include "util/c_include_parser.h"

#define DEFAULT_SETTINGS_FILE "includer.settings"

namespace fs = std::filesystem;

struct hash_t {
  std::map<std::string, std::vector<std::string>> compiler_include;
  std::map<std::string, std::vector<std::string>> third_include;
  std::map<std::string, std::vector<std::string>> project_files;
};

class includer {
  bool validate_args() noexcept;
  bool get_settings() RELEASE_noexcept;
  void hash_files() noexcept;
  bool parse_source() RELEASE_noexcept;
  bool parse_file(const fs::path &file) RELEASE_noexcept;
  void fix_includes() noexcept;

  void hash_group(std::vector<fs::path> &group, std::map<std::string, std::vector<std::string>> &hash) noexcept;

  enum class print_t {
    good,
    collision,
    not_found,
    auto_fix,
    manual,
    source,
  };

  static std::map<print_t, std::string> print_m;
  std::map<print_t, unsigned long long> print_counter;

  static void find_in_dirs(const fs::path &include,
                           const std::vector<fs::path> &dirs,
                           std::vector<fs::path> &selection) noexcept;
  static std::string replace_slash(std::string path, bool &overwrite) noexcept;
  static void print_inc(const std::string &file,
                 const include_t &inc_orig,
                 const print_t &type,
                 const std::vector<include_t> *inc_new = nullptr) RELEASE_noexcept;
  static void variants(const fs::path &file,
                       const std::vector<fs::path> &selection,
                       std::vector<include_t> &vars,
                       const bool &exp) noexcept;

  const int &argc;
  char **argv;

  c_include_parser parser;
  std::map<std::string, std::vector<include_t>> full_includes;
  hash_t hashs;
  fs::path settings_file;
  std::ofstream *warning_stream = nullptr;
  settings set;
  static const std::string help;  // TODO(kmosk): set help
 public:
  includer(const int &argc, char *argv[]);
  ~includer();
  bool run();
};

#endif //INCLUDER_H_CBAF2912A3AC436FABCBFFD4BC185A35
