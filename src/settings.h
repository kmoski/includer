#ifndef SETTINGS_H_3A1EC39BF259498DA55A11CBB0DC6F90
#define SETTINGS_H_3A1EC39BF259498DA55A11CBB0DC6F90

#include <filesystem>
#include <vector>
#include <map>

#define SET_PROJECT_DIRECTORY   "project_directory"
#define SET_COMPILER_INCLUDE    "compiler_include"
#define SET_THIRD_INCLUDE       "third_include"
#define SET_PROJECT_INCLUDE     "project_include"
#define SET_SOURCE              "source"
#define SET_DEFINES             "defines"
#define SET_EXTENSIONS_INCLUDE  "extensions_include"
#define SET_EXTENSIONS_SOURCE   "extensions_source"
#define SET_FIX_INCLUDE         "fix_include"
#define SET_FIX_CASE            "fix_case"
#define SET_SHOW_NIEF           "show_nief"

#define SET_C_NONE       "none"
#define SET_C_LOWER      "lower"
#define SET_C_FILESYSTEM "filesystem"

#define SET_TRUE  "true"
#define SET_FALSE "false"

#define SET_SEPARATOR ' '

namespace fs = std::filesystem;

/**
 * @brief // TODO(kmosk):
 */
struct settings {
  enum class case_e : unsigned char {  /// @brief types of case conversation
    none,        ///> not convert include string case
    lower,       ///> convert include string to lower case
    filesystem,  ///> convert include string as named file path in filesystem
    err,         ///> success state
  };

  // project settings
  fs::path project_directory;              ///> path to root directory of project (dir)
  std::vector<fs::path> compiler_include;  ///> paths to compiler include directories (dirs)
  std::vector<fs::path> third_include;     ///> paths to third-party include directories (dirs)
  std::vector<fs::path> project_include;   ///> paths to current project include directories (dirs)
  std::vector<fs::path> source;            ///> paths to compiling source files (files)
  std::vector<std::string> defines;        ///> defines for preprocessing (only for project full_includes)
  std::vector<std::string> extensions_include;
  std::vector<std::string> extensions_source;

  // fix settings
  case_e fix_case;                        ///> convert case in include string
  bool fix_include;                       ///> enable fix full_includes
  bool show_not_include_extension_files;  ///> enable show includes with not extensions_include

  [[nodiscard]] bool case_enable() const noexcept;   ///> enable convert case in include string

  void clear() noexcept;  /// @brief clear struct and set default values
  void print(std::ostream &out) const noexcept;  /// @brief print struct to stream

  static const std::map<const settings::case_e, const std::string> case_m;    ///> search map for string of case_e
  static settings::case_e get_case(const std::string &str) noexcept;    /// @brief return case setting by string
};

#endif //SETTINGS_H_3A1EC39BF259498DA55A11CBB0DC6F90
