#include <iostream>
#include <cassert>
#include "includer.h"
#include "output.h"
#include "util/settings_parser.h"

const std::string includer::help = // NOLINT(cert-err58-cpp)
    "Using:\n"
    "\tincluder [settings file] [warning file]\n"
    "\n"
    "\tsettings file default is \"" DEFAULT_SETTINGS_FILE "\"\n"
    "\t  note: you can set dir:\n"
    "\t  \"..\"\n"
    "\t  and find settings file path is:\n"
    "\t  \"../" DEFAULT_SETTINGS_FILE "\""
    "\twarning file default is standard output";
std::map<includer::print_t, std::string> includer::print_m = {
    { print_t::good,      "good     " },
    { print_t::collision, "collision" },
    {print_t::not_found,  "not_found" },
    { print_t::auto_fix,  "auto_fix " },
    {print_t::manual,     "manual   " },
    { print_t::source,    "source   " },
};
includer::includer(const int &argc, char **argv)
    : argc(argc), argv(argv) {}
includer::~includer() {
  output::destroy();
  if (warning_stream != nullptr) {
    warning_stream->close();
  }
  delete warning_stream;
}
bool includer::run() {
  if (!validate_args()) {
    return false;
  }
  if (!get_settings()) {
    return false;
  }
  set.print(to_log);
  // hash_files();
  if (!parse_source()) {
    return false;
  }
  if (set.fix_include) {
    fix_includes();
  }

  // show all used dirs
  // auto full_copy = full_includes;
  // std::vector<std::string> used;
  // for (auto &p : full_includes) {
  //   used.push_back(fs::path(p.first).parent_path().string());
  //   to_log << p.first << endl;
  // }
  // std::sort(used.begin(), used.end());
  // auto end = std::unique(used.begin(), used.end());
  // used.erase(end, used.end());
  // to_log << "start" << endl;
  // for (auto &p : used) {
  //   to_log << p << endl;
  // }

  for (auto &p : print_counter) {
    to_log << print_m[p.first] << ' ' << p.second << endl;
  }
  return true;
}
bool includer::validate_args() noexcept {
  if (argc > 3) {
    std::cerr << "Incorrect number of argument" << std::endl;
    std::cerr << help << std::endl;
    return false;
  }
  if (argc > 1) {  // set settings
    settings_file = argv[1];
    if (!fs::exists(settings_file)) {
      std::cerr << "Settings file does not exist ("
                << settings_file << ')' << std::endl;
      std::cerr << help << std::endl;
      return false;
    }
    if (fs::is_directory(settings_file)) {
      settings_file /= DEFAULT_SETTINGS_FILE;
    }
  } else {
    settings_file = ".";
    settings_file /= DEFAULT_SETTINGS_FILE;
  }
  if (argc > 2) {  // set warning
    warning_stream = new std::ofstream(argv[2]);
    if (!warning_stream->good()) {
      std::cerr << "Warning stream not good =)" << std::endl;
      std::cerr << help << std::endl;
      return false;
    }
    output::init(std::cout, *warning_stream, std::cerr);
  } else {
    output::init(std::cout, std::cout, std::cerr);
  }
  return true;
}
bool includer::get_settings() RELEASE_noexcept {
  settings_parser set_parser;
  if (!set_parser.from_file(settings_file, &set)) {
    return false;
  }
  return true;
}
void includer::hash_files() noexcept {
  hash_group(set.compiler_include, hashs.compiler_include);
  hash_group(set.third_include, hashs.third_include);
  hash_group(set.project_include, hashs.project_files);
  hash_group(set.source, hashs.project_files);
}
void includer::hash_group(std::vector<fs::path> &group, // NOLINT(readability-make-member-function-const)
                          std::map<std::string, std::vector<std::string>> &hash) noexcept {
  for (auto &inc : group) {
    fs::path full;
    if (inc.has_root_directory()) {
      full = inc.lexically_normal();
    } else {
      full = (set.project_directory / inc).lexically_normal();
    }
    if (fs::is_directory(full)) {
      // dir
      for (auto &i : fs::recursive_directory_iterator(full)) {
        if (!fs::is_directory(i)) {
          auto &tmp = i.path();
          auto &paths = hash[tmp.filename().string()];
          auto tmp_s = tmp.string();
          bool exist = false;
          for (auto &path : paths) {
            if (tmp_s == path) {
              exist = true;
              break;
            }
          }
          if (!exist) {
            paths.push_back(tmp_s);
          }
        }
      }
    } else {
      // file
      auto &paths = hash[full.filename().string()];
      auto tmp_s = full.string();
      bool exist = false;
      for (auto &path : paths) {
        if (tmp_s == path) {
          exist = true;
          break;
        }
      }
      if (!exist) {
        paths.push_back(tmp_s);
      }
    }
  }
}
bool includer::parse_source() RELEASE_noexcept {
  for (auto &src : set.source) {
    if (fs::is_directory(src)) {
      //dir
      for (auto &file_entry : fs::recursive_directory_iterator(src)) {
        auto &file = file_entry.path();
        auto extension = file.extension();
        bool verify_extension = false;
        for (auto &ext : set.extensions_source) {
          if (extension.string() == ext) {
            verify_extension = true;
            break;
          }
        }
        if (verify_extension) {
          if (!parse_file(file)) {
            return false;
          }
        }
      }
    } else {
      //file
      if (!parse_file(src)) {
        return false;
      }
    }
  }
  return true;
}
bool includer::parse_file(const fs::path &file) RELEASE_noexcept {
  if (full_includes.find((file.string())) != full_includes.end()) {
    return true;
  }
  assert(file.has_root_directory());
  auto &includes = full_includes[file];
  if (!parser.from_file(&file, &includes)) {
    return false;
  }
  for (auto &inc : includes) {
    inc.path = replace_slash(inc.path.string(), inc.overwrite);
    if (set.show_not_include_extension_files) {
      bool is_source = true;
      for (auto &extension : set.extensions_include) {
        auto ext = inc.path.extension().string();
        if (ext == extension ||
            ext.empty()) {  // TODO(kmosk): files without extension, like #include <string>
          is_source = false;
        }
      }
      if (is_source) {
        print_counter[print_t::source]++;
        print_inc(file, inc, print_t::source);
      }
    }
    if (inc.exp) {  // now ""
      // find full-relative path
      auto inc_good_full_path = (file.parent_path() / inc.path).lexically_normal();
      if (fs::exists(inc_good_full_path)) {  // full-relative path
        // TODO(kmosk): if include "Hello.txt", but really filename "hEllo.txt", std::exists() return false in
        //  case-sensitive filesystem
        print_counter[print_t::good]++;
        if (!parse_file(inc_good_full_path)) {
          return false;
        }
      } else {  // not full-relative
        // find with appended to include dirs
        std::vector<fs::path> selection;
        std::vector<fs::path> selection_not_exp;
        find_in_dirs(inc.path, set.project_include, selection);
        find_in_dirs(inc.path, set.compiler_include, selection_not_exp);
        find_in_dirs(inc.path, set.third_include, selection_not_exp);
        if (selection.empty() && selection_not_exp.empty()) {  // not found
          // unreal phenomenon
          print_counter[print_t::not_found]++;
          print_inc(file, inc, print_t::not_found);
        } else if (selection.size() == 1 && selection_not_exp.empty()) {  // found in include dirs
          // auto fix that
          print_counter[print_t::auto_fix]++;
          include_t new_inc;
          new_inc.path = selection.at(0);
          new_inc.path = new_inc.path.lexically_relative(file.parent_path());
          new_inc.exp = true;
          std::vector<include_t> var = {new_inc};
          // print_inc(file, inc, print_t::auto_fix, &var);  // TODO(kmosk): uncomment

          inc.path = new_inc.path;
          inc.overwrite = true;
          if (!parse_file(selection.at(0))) {
            return false;
          }
        } else if (selection_not_exp.size() == 1 && selection.empty()) {  // found in compiler or third dirs
          // overwrite as not exp include
          print_counter[print_t::manual]++;
          include_t new_inc;
          new_inc.path = selection_not_exp.at(0);
          new_inc.exp = false;
          std::vector<include_t> var = {new_inc};
          print_inc(file, inc, print_t::manual, &var);
        } else {  // collision
          // manual fix
          std::vector<include_t> vars{};
          variants(file, selection, vars, true);
          variants(file, selection_not_exp, vars, false);
          print_counter[print_t::collision]++;
          print_inc(file, inc, print_t::collision, &vars);
        }
      }
    } else {  // now <>
      // find with appended to include dirs
      std::vector<fs::path> selection;
      std::vector<fs::path> selection_exp;
      find_in_dirs(inc.path, set.compiler_include, selection);
      find_in_dirs(inc.path, set.third_include, selection);
      find_in_dirs(inc.path, set.project_include, selection_exp);
      if (selection.empty() && selection_exp.empty()) {  // not found
        // unreal phenomenon
        print_counter[print_t::not_found]++;
        print_inc(file, inc, print_t::not_found);
      } else if (selection.size() == 1 && selection_exp.empty()) {  // found in compiler or third dirs
        print_counter[print_t::good]++;
      } else if (selection_exp.size() == 1 && selection.empty()) {  // found in include dirs
        // overwrite as exp include and run parse
        print_counter[print_t::manual]++;
        include_t new_inc;
        new_inc.path = selection_exp.at(0);
        new_inc.path = new_inc.path.lexically_relative(file.parent_path());
        new_inc.exp = true;
        std::vector<include_t> var = {new_inc};
        print_inc(file, inc, print_t::manual, &var);

        // inc.path = new_inc.path;
        // inc.overwrite = true;
        // if (!parse_file(selection_exp.at(0))) {
        //   return false;
        // }
        // TODO(kmosk): uncomment this
      } else {  // collision
        // manual fix
        std::vector<include_t> vars{};
        variants(file, selection, vars, false);
        variants(file, selection_exp, vars, true);
        print_counter[print_t::collision]++;
        print_inc(file, inc, print_t::collision, &vars);
      }
    }
  }
  return true;
}
void includer::print_inc(const std::string &file,
                         const include_t &inc_orig,
                         const print_t &type,
                         const std::vector<include_t> *inc_new) RELEASE_noexcept {
  to_log << print_m[type] << ' ' << file << ':' << inc_orig.line
         << ": " << (inc_orig.exp ? '\"' : '<')
                 << inc_orig.path.string()
                 << (inc_orig.exp ? '\"' : '>');
  switch (type) {
    case print_t::collision: {
      assert(inc_new != nullptr);
      to_log << " variants:" << endl;
      for (auto &inc : *inc_new) {
        to_log << '\t' << (inc.exp ? '\"' : '<')
                       << inc.path.string()
                       << (inc.exp ? '\"' : '>') << endl;
      }
      break;
    }
    case print_t::good:
    case print_t::source:
    case print_t::not_found: {
      to_log << endl;
      break;
    }
    case print_t::manual:
    case print_t::auto_fix: {
      assert(inc_new != nullptr);
      assert(inc_new->size() == 1);
      to_log << " to " << ((*inc_new)[0].exp ? '\"' : '<')
                       << (*inc_new)[0].path.string()
                       << ((*inc_new)[0].exp ? '\"' : '>') << endl;
      break;
    }
  }
}
void includer::variants(const fs::path &file,
                        const std::vector<fs::path> &selection,
                        std::vector<include_t> &vars,
                        const bool &exp) noexcept {
  for (auto &variant : selection) {
    include_t inc;
    inc.exp = exp;
    if (exp) {
      inc.path = variant.lexically_relative(file);
    } else {
      // TODO(kmosk): set path relative from compiler or third dirs
      inc.path = variant;
    }
    vars.push_back(inc);
  }
}
void includer::fix_includes() noexcept {
  std::vector<char> file_buffer{};
  for (auto &p : full_includes) {
    file_buffer.clear();
    char tmp_in;
    std::fstream file(p.first, std::ios_base::binary | std::ios_base::in);
    while(file.read(&tmp_in, 1)) {
      file_buffer.push_back(tmp_in);
    }
    file.close();
    int offset = 0;
    for (auto &inc : p.second) {
      if (inc.overwrite) {
        auto begin = file_buffer.begin() + inc.position + offset;
        auto end = begin + inc.length;
        file_buffer.erase(begin, end);
        auto iter = file_buffer.begin() + inc.position + offset - 1;
        file_buffer.insert(++iter, (inc.exp ? '\"' : '<'));
        for (auto &c : inc.path.string()) {
          file_buffer.insert(++iter, c);
        }
        file_buffer.insert(++iter, (inc.exp ? '\"' : '>'));
        offset += int(inc.path.string().length()) - int(inc.length) + 2;
      }
    }
    file.open(p.first, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    for (auto &tmp_out : file_buffer) {
      file.write(&tmp_out, 1);
    }
    file.flush();
    file.close();
  }
}
std::string includer::replace_slash(std::string path, bool &overwrite) noexcept {
  unsigned long pos;
  while ((pos = path.find("\\\\")) != std::string::npos) {
    path.replace(pos, 2, "/");
    overwrite = true;
  }
  while ((pos = path.find('\\')) != std::string::npos) {
    path.replace(pos, 1, "/");
    overwrite = true;
  }
  return std::move(path);
}
void includer::find_in_dirs(const fs::path &include,
                            const std::vector<fs::path> &dirs,
                            std::vector<fs::path> &selection) noexcept {
  for (auto &dir : dirs) {
    auto possible = dir / include;
    if (fs::exists(possible)) {
      selection.push_back(possible);
    }
  }
}
