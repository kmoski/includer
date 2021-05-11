#ifndef SETTINGS_PARSER_H_8FB41AE2E6AE48449AE7EEE9AC7B28E4
#define SETTINGS_PARSER_H_8FB41AE2E6AE48449AE7EEE9AC7B28E4

#include <filesystem>
#include <vector>
#include <fstream>
#include "../settings.h"
#include "../defines.h"

namespace fs = std::filesystem;

/**
 * @brief // TODO(kmosk):
 */
class settings_parser {
  bool continue_parse() RELEASE_noexcept;
  template<typename T>
  bool get_one_param(T &param, const std::string &error_msg) RELEASE_noexcept;
  template<typename T>
  bool get_few_param(std::vector<T> &param, const std::string &error_msg, const bool &raw) RELEASE_noexcept;
  bool get_bool_param(bool &param, const std::string &error_msg) RELEASE_noexcept;
  bool get_setting_case() noexcept;

  template<typename T>
  static void parse_paths(std::vector<T> &arr, const std::string &input, const char &separator) noexcept;
  template<typename T>
  static void split(std::vector<T> &arr, const std::string &input, const char &separator) noexcept;

  static void send_error(const std::string &msg) RELEASE_noexcept;
  static void send_error_syntax(const std::string &msg) RELEASE_noexcept;
  static void send_error_eof(const std::string &msg) RELEASE_noexcept;

  settings *s;
  std::ifstream file;
  std::string tmp_s;

  bool verify() RELEASE_noexcept;
  bool verify_paths(std::vector<fs::path> &paths,
                    const std::string &error_msg,
                    bool check_dir = true) RELEASE_noexcept;
  static void verify_extensions(std::vector<std::string> &extensions) noexcept;
 public:
  bool from_file(const fs::path &path, settings *set) RELEASE_noexcept;
};

#endif //SETTINGS_PARSER_H_8FB41AE2E6AE48449AE7EEE9AC7B28E4
