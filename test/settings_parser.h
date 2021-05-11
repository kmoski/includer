#ifndef SETTINGS_PARSER_H_348D478FDBCD47EAA4201DDA0E891032
#define SETTINGS_PARSER_H_348D478FDBCD47EAA4201DDA0E891032

#include <gtest/gtest.h>
#include "../src/util/settings_parser.h"

class settings_parser_test
    : public ::testing::Test {
  std::vector<fs::path> temp_fs;
 public:
  void SetUp() override;
  void TearDown() override;
  void create_temp_file(const fs::path &path, const std::string &data);
  void create_dir(const fs::path &path);

  settings_parser parser;
};

#endif //SETTINGS_PARSER_H_348D478FDBCD47EAA4201DDA0E891032
