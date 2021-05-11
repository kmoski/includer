#include "settings_parser.h"
#include "../output.h"

bool settings_parser::from_file(const fs::path &path, settings *set) RELEASE_noexcept {
  s = set;
  if (!fs::exists(path)) {
    send_error("Settings file does not exist ("
                   + path.string() + ')');
    return false;
  }
  file.open(path);
  s->clear();
  while (file >> tmp_s) {
    if (!continue_parse()) {
      return false;
    }
  }
  file.close();
  if (!verify()) {
    return false;
  }
  return true;
}
bool settings_parser::continue_parse() RELEASE_noexcept {
  if (tmp_s == SET_PROJECT_DIRECTORY) {
    return get_one_param(s->project_directory, SET_PROJECT_DIRECTORY);
  } else if (tmp_s == SET_COMPILER_INCLUDE) {
    return get_few_param(s->compiler_include, SET_COMPILER_INCLUDE, false);
  } else if (tmp_s == SET_THIRD_INCLUDE) {
    return get_few_param(s->third_include, SET_THIRD_INCLUDE, false);
  } else if (tmp_s == SET_PROJECT_INCLUDE) {
    return get_few_param(s->project_include, SET_PROJECT_INCLUDE, false);
  } else if (tmp_s == SET_SOURCE) {
    return get_few_param(s->source, SET_SOURCE, false);
  } else if (tmp_s == SET_DEFINES) {
    return get_few_param(s->defines, SET_DEFINES, true);
  } else if (tmp_s == SET_EXTENSIONS_INCLUDE) {
    return get_few_param(s->extensions_include, SET_EXTENSIONS_INCLUDE, true);
  } else if (tmp_s == SET_EXTENSIONS_SOURCE) {
    return get_few_param(s->extensions_source, SET_EXTENSIONS_SOURCE, true);
  } else if (tmp_s == SET_FIX_INCLUDE) {
    return get_bool_param(s->fix_include, SET_FIX_INCLUDE);
  } else if (tmp_s == SET_FIX_CASE) {
    return get_setting_case();
  } else if (tmp_s == SET_SHOW_NIEF) {
    return get_bool_param(s->show_not_include_extension_files, SET_SHOW_NIEF);
  } else {
    send_error_syntax("Unknown param: " + tmp_s);
    return false;
  }
}
template<typename T>
bool settings_parser::get_one_param(T &param, const std::string &error_msg) RELEASE_noexcept {
  if (std::getline(file, tmp_s)) {
    if (tmp_s.empty()) {
      send_error_syntax("Empty argument: " + error_msg);
      return false;
    }
    tmp_s = tmp_s.substr(1);
    param = tmp_s;
    return true;
  } else {
    send_error_eof(error_msg);
    return false;
  }
}
template<typename T>
bool settings_parser::get_few_param(std::vector<T> &param,
                                    const std::string &error_msg,
                                    const bool &raw) RELEASE_noexcept {
  if (std::getline(file, tmp_s)) {
    if (tmp_s.empty()) {
      send_error_syntax("Empty argument: " + error_msg);
      return false;
    }
    tmp_s = tmp_s.substr(1);
    if (raw) {
      split(param, tmp_s, SET_SEPARATOR);
    } else {
      parse_paths(param, tmp_s, SET_SEPARATOR);
    }
    return true;
  } else {
    send_error_eof(error_msg);
    return false;
  }
}
bool settings_parser::get_bool_param(bool &param, const std::string &error_msg) {
  if (file >> tmp_s) {
    if (tmp_s == SET_TRUE) {
      param = true;
      return true;
    } else if (tmp_s == SET_FALSE) {
      param = false;
      return true;
    } else {
      send_error_syntax(std::string("Expect ") + SET_TRUE + " or " + SET_FALSE ": " + tmp_s);
      return false;
    }
  } else {
    send_error_eof(error_msg);
    return false;
  }
}
bool settings_parser::get_setting_case() noexcept {
  if (file >> tmp_s) {
    auto state = settings::get_case(tmp_s);
    if (state == settings::case_e::err) {
      send_error_syntax(std::string("Expect [") + SET_C_NONE + ", "
                                                + SET_C_LOWER ", "
                                                + SET_C_FILESYSTEM"]: " + tmp_s);
      return false;
    } else {
      s->fix_case = state;
      return true;
    }
  } else {
    send_error_eof(SET_FIX_CASE);
    return false;
  }
}
template<typename T>
void settings_parser::parse_paths(std::vector<T> &arr, const std::string &input, const char &separator) noexcept {
  std::string arg;
  bool quot = false;
  bool control = false;
  for (auto &c : input) {
    if (control) {
      arg.push_back(c);
      control = false;
      continue;
    }
    switch (c) {
      case '\"': {
        quot = !quot;
        break;
      }
      case '\\': {
        control = true;
        break;
      }
      default: {
        if (c == separator) {
          if (!quot) {
            arr.emplace_back(arg);
            arg.clear();
            break;
          }
        }
        arg.push_back(c);
        break;
      }
    }
  }
  arr.emplace_back(arg);
}
template<typename T>
void settings_parser::split(std::vector<T> &arr, const std::string &input, const char &separator) noexcept {
  size_t pos = -1;
  size_t num = 0;
  while ((pos = input.find(separator, pos + 1)) != std::string::npos) {
    arr.emplace_back(input.substr(num, pos - num));
    num = pos + 1;
  }
  {
    auto len = input.length() - num;
    if (num != len) {
      arr.emplace_back(input.substr(num, len));
    }
  }
}
void settings_parser::send_error(const std::string &msg) RELEASE_noexcept {
  to_error << "Settings parser: " << msg << endl;
}
void settings_parser::send_error_syntax(const std::string &msg) RELEASE_noexcept {
  send_error("Wrong syntax: " + msg);
}
void settings_parser::send_error_eof(const std::string &msg) RELEASE_noexcept {
  send_error("Unexpected end of file: " + msg);
}
bool settings_parser::verify() RELEASE_noexcept {
  if (!fs::exists(s->project_directory) ||
      !fs::is_directory(s->project_directory)) {
    send_error("Incorrect project directory: " + s->project_directory.string());
    return false;
  }
  if (s->project_directory.has_root_directory()) {
    s->project_directory = s->project_directory.lexically_normal();
  } else {
    s->project_directory = (fs::current_path() / s->project_directory).lexically_normal();
  }
  if (!verify_paths(s->compiler_include, "Incorrect compiler include")) {
    return false;
  }
  if (!verify_paths(s->third_include, "Incorrect third include")) {
    return false;
  }
  if (!verify_paths(s->project_include, "Incorrect project include")) {
    return false;
  }
  if (!verify_paths(s->source, "Incorrect source", false)) {
    return false;
  }
  verify_extensions(s->extensions_include);
  verify_extensions(s->extensions_source);
  return true;
}
bool settings_parser::verify_paths(std::vector<fs::path> &paths,
                                   const std::string &error_msg,
                                   bool check_dir) RELEASE_noexcept {
  for (auto &current : paths) {
    fs::path path;
    auto has_root = current.has_root_directory();
    if (has_root) {
      path = current;
    } else {
      path = (s->project_directory / current);
    }
    if (!fs::exists(path) ||
        (check_dir && !fs::is_directory(path))) {
      send_error(error_msg + ": " + current.string());
      return false;
    } else {
      current = path.lexically_normal();
    }
  }
  std::sort(paths.begin(), paths.end());
  auto end = std::unique(paths.begin(), paths.end());
  paths.erase(end, paths.end());
  return true;
}
void settings_parser::verify_extensions(std::vector<std::string> &extensions) noexcept {
  for (auto &ext : extensions) {
    if (ext[0] != '.') {
      ext.insert(0, ".");
    }
  }
  std::sort(extensions.begin(), extensions.end());
  auto end = std::unique(extensions.begin(), extensions.end());
  extensions.erase(end, extensions.end());
}
