#include "settings.h"
#include "output.h"

const std::map<const settings::case_e, const std::string> settings::case_m = {  // NOLINT(cert-err58-cpp)
    {settings::case_e::none, SET_C_NONE },
    {settings::case_e::lower, SET_C_LOWER },
    {settings::case_e::filesystem, SET_C_FILESYSTEM },
};
bool settings::case_enable() const noexcept {
  return fix_case != case_e::none;
}
void settings::clear() noexcept {
  project_directory.clear();
  compiler_include.clear();
  third_include.clear();
  project_include.clear();
  source.clear();
  defines.clear();
  fix_include = false;
  fix_case = case_e::filesystem;
  show_not_include_extension_files = true;
}
void settings::print(std::ostream &out) const noexcept {
  out << "Project directory:" << endl
      << "  " << project_directory.string() << endl
      << endl
      << "Compiler include:" << endl;
  for (auto &inc : compiler_include) {
    out << "  " << inc.string() << endl;
  }
  out << endl
      << "Third include:" << endl;
  for (auto &inc : third_include) {
    out << "  " << inc.string() << endl;
  }
  out << endl
      << "Project include:" << endl;
  for (auto &inc : project_include) {
    out << "  " << inc.string() << endl;
  }
  out << endl
      << "Source:" << endl;
  for (auto &src : source) {
    out << "  " << src.string() << endl;
  }
  out << endl
      << "Defines:" << endl;
  for (auto &def : defines) {
    out << "  " << def << endl;
  }
  out << endl
      << "Include extensions:" << endl;
  for (auto &ext : extensions_include) {
    out << "  " << ext << endl;
  }
  out << endl
      << "Source extensions:" << endl;
  for (auto &ext : extensions_source) {
    out << "  " << ext << endl;
  }
  out << endl
      << "Fix include:             " << (fix_include         ? "yes" : "no") << endl
      << "Fix case:                " << (case_enable()       ? "yes" : "no") << endl
      << "Show include source:     " << (show_not_include_extension_files ? "yes" : "no") << endl;
  if (case_enable()) {
    out << endl;
    out << "Type case:  " << case_m.at(fix_case) << endl;
  }
}
settings::case_e settings::get_case(const std::string &str) noexcept {
  if (str == SET_C_NONE) {
    return settings::case_e::none;
  } else if (str == SET_C_LOWER) {
    return settings::case_e::lower;
  } else if (str == SET_C_FILESYSTEM) {
    return settings::case_e::filesystem;
  } else {
    return settings::case_e::err;
  }
}
