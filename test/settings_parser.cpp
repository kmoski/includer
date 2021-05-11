#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include <gtest/gtest.h>
#include "settings_parser.h"
#include "../src/output.h"

void settings_parser_test::SetUp() {
  output::init(std::cout, std::cout, std::cout);
}
void settings_parser_test::TearDown() {
  for (auto &temp : temp_fs) {
    try {
      fs::remove_all(temp);
    } catch (...) {}
  }
  output::destroy();
}
void settings_parser_test::create_temp_file(const fs::path &path, const std::string &data) {
  std::ofstream file(path);
  file << data;
  file.close();
  temp_fs.push_back(path);
}
void settings_parser_test::create_dir(const fs::path &path) {
  fs::create_directory(path);
  temp_fs.push_back(path);
}

TEST_F(settings_parser_test, simple) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_COMPILER_INCLUDE " cinc1 ./cinc2\n"
                   SET_THIRD_INCLUDE " tinc1 \"./cinc1/../tinc 2\"\n"
                   SET_PROJECT_INCLUDE " pinc1 pinc2\n"
                   SET_SOURCE " s1 s2\n"
                   SET_DEFINES " NDEBUG DEBUG\n"
                   SET_WARNING " true\n"
                   SET_EXTENSIONS_INCLUDE " .h .hpp\n"
                   SET_EXTENSIONS_SOURCE " .c .cpp\n"
                   SET_FIX_INCLUDE " true\n"
                   SET_FIX_SLASH " none\n"
                   SET_FIX_CASE " none\n"
                   SET_SHOW_NIEF " false");
  create_dir(filename);
  create_dir(filename + "/cinc1");
  create_dir(filename + "/cinc2");
  create_dir(filename + "/tinc1");
  create_dir(filename + "/tinc 2");
  create_dir(filename + "/pinc1");
  create_dir(filename + "/pinc2");
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_TRUE(parser.from_file(filename + ".cfg", &set));
  ASSERT_EQ(set.project_directory, "./" + filename);
  ASSERT_EQ(set.compiler_include[0], "cinc1");
  ASSERT_EQ(set.compiler_include[1], "./cinc2");
  ASSERT_EQ(set.third_include[0], "tinc1");
  ASSERT_EQ(set.third_include[1], "./cinc1/../tinc 2");
  ASSERT_EQ(set.project_include[0], "pinc1");
  ASSERT_EQ(set.project_include[1], "pinc2");
  ASSERT_EQ(set.source[0], "s1");
  ASSERT_EQ(set.source[1], "s2");
  ASSERT_EQ(set.defines[0], "NDEBUG");
  ASSERT_EQ(set.defines[1], "DEBUG");
  ASSERT_TRUE(set.warning);
  ASSERT_EQ(set.extensions_include[0], ".h");
  ASSERT_EQ(set.extensions_include[1], ".hpp");
  ASSERT_EQ(set.extensions_source[0], ".c");
  ASSERT_EQ(set.extensions_source[1], ".cpp");
  ASSERT_TRUE(set.fix_include);
  ASSERT_FALSE(set.slash_enable());
  ASSERT_FALSE(set.case_enable());
  ASSERT_FALSE(set.show_not_include_extension_files);
}

TEST_F(settings_parser_test, minimal) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n"
                   SET_EXTENSIONS_INCLUDE " h .hpp\n"
                   SET_EXTENSIONS_SOURCE " .c cpp\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_TRUE(parser.from_file(filename + ".cfg", &set));
  ASSERT_EQ(set.extensions_include[0], ".h");
  ASSERT_EQ(set.extensions_include[1], ".hpp");
  ASSERT_EQ(set.extensions_source[0], ".c");
  ASSERT_EQ(set.extensions_source[1], ".cpp");
}

TEST_F(settings_parser_test, extensions) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_TRUE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, project_dir_quoted) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " \"./space " + filename + "\"\n"
                   SET_SOURCE " s1 s2\n");
  create_dir("space " + filename);
  create_dir("space " + filename + "/s1");
  create_dir("space " + filename + "/s2");
  settings set;
  ASSERT_TRUE(parser.from_file(filename + ".cfg", &set));
  ASSERT_EQ(set.project_directory, "./space " + filename);
}

TEST_F(settings_parser_test, without_project_dir) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_SOURCE " s1 s2\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, without_source) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n");
  create_dir(filename);
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, double_project_dir) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, double_warning) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n"
                   SET_WARNING " true\n"
                   SET_WARNING " true\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, double_fix_include) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n"
                   SET_FIX_INCLUDE " true\n"
                   SET_FIX_INCLUDE " true\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, double_show_source_include) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n"
                   SET_SHOW_NIEF " false"
                   SET_SHOW_NIEF " false");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, double_fix_slash) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n"
                   SET_FIX_SLASH " none\n"
                   SET_FIX_SLASH " none\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

TEST_F(settings_parser_test, double_fix_case) {
  std::string filename = test_info_->name();
  create_temp_file(filename + ".cfg",
                   SET_PROJECT_DIRECTORY " ./" + filename + "\n"
                   SET_SOURCE " s1 s2\n"
                   SET_FIX_CASE " none\n"
                   SET_FIX_CASE " none\n");
  create_dir(filename);
  create_dir(filename + "/s1");
  create_dir(filename + "/s2");
  settings set;
  ASSERT_FALSE(parser.from_file(filename + ".cfg", &set));
}

#pragma clang diagnostic pop