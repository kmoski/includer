#include "includer.h"
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;
using std::move;

static const char *include_string = "include";
static const auto buf_len = strlen(include_string);

includer::includer(const int &argc, char **argv) {
  // if (argc < 3) {
  if (argc < 2) {
    cout << "Incorrect argc:" << endl;
    cout << "argc = " << argc << endl;
    throw;
  }
  main_dir = argv[1];
  if (!fs::exists(main_dir) || !fs::is_directory(main_dir)) {
    cout << "Incorrect main directory:" << endl;
    cout << main_dir << endl;
    throw;
  }
  // for (unsigned i{2}; i < argc; ++i) {
  //   include_dirs.emplace_back(argv[i]);
  //   if (!fs::exists(include_dirs.back()) ||
  //       !fs::is_directory(include_dirs.back())) {
  //     cout << "Incorrect include directory:" << endl;
  //     cout << include_dirs.back() << endl;
  //     throw;
  //   }
  // }
}
void includer::recursive_initialization() noexcept {
  for (auto &i : fs::recursive_directory_iterator(main_dir)) {
    i_fs_file = &i;
    auto ext = i.path().extension();
    if (ext == ".cpp" || ext == ".h") {
      line = 1;
      i_file.open(i.path());
      // cout << endl << "passed" << i.path() << endl;
      while (!i_file.eof()) {
        tmp_c = i_file.peek();
        if (!continue_parse()) {
          break;
        }
      }
      i_file.close();
    }
  }
}
bool includer::continue_parse() noexcept {
  switch (tmp_c) {
    case '/': {  // exclude find in comment
      return read_next("Read after '/'") &&
             parse_comment();
    }
    case '\"': {  // exclude find in string
      return read_next("Read after '\"'") &&
             parse_string();
    }
    case '#': {
      return read_next("Read after '#'") &&
             parse_directive();
    }
    case '\'': {
      if (!read_next("Read symbols, after '")) {  // read '
        return false;
      }
      tmp_c = i_file.peek();
      switch (tmp_c) {
        case '\\': {
          if (!read_next("Read symbols, after \\")) {  // read '\'
            return false;
          }
          if (!read_next("Read symbols, after " + std::to_string(tmp_c))) {  // read control symbol
            return false;
          }
          break;
        }
        case '#':
        case '/':
        default: {
          if (!read_next("Read symbols, after " + std::to_string(tmp_c))) {  // read any symbol
            return false;
          }
          break;
        }
      }
      if (!read_next("Read symbols, after '")) {  // read '
        return false;
      }
      break;
    }
    case '\n':
    case '\r': {
      // no need to get a character from the stream
      parse_end_line(msg_type::none);
      if (endl_default) {
        return false;
      }
      return true;
    }
    default: {
      return read_next("Continue parse", msg_type::none);
    }
  }
  return true;
}
bool includer::parse_comment() noexcept {
  tmp_c = i_file.peek();
  switch (tmp_c) {
    case '/': {  // exclude find in line comment
      bool end_comment = false;
      if (!read_next(R"(Read after "//")")) {
        return false;
      }
      while (!end_comment) {
        tmp_c = i_file.peek();
        switch (tmp_c) {
          case '\n':
          case '\r': {  // end line comment
            parse_end_line(msg_type::none);
            if (endl_default) {
              return false;
            }
            end_comment = true;
            break;
          }
          case '\\': {  // multi-line comment
            if (!read_next(R"(Read comment after "\")", msg_type::warning)) {
              return false;
            }
            if (!parse_end_line(msg_type::warning, false)) {
              return false;
            }
            break;
          }
          default: {
            if (!read_next("Read comment without end line", msg_type::warning)) {
              return false;
            }
            break;
          }
        }
      }
      break;
    }
    case '*': {  // exclude find in block comment
      if (!read_next(R"(Read after "/*")")) {
        return false;
      }
      if (!parse_block_comment()) {
        return false;
      }
      break;
    }
    default: {
      // message("??? syntax", "Expect '/' or '*' after '/'", msg_type::mosk);
      return true;
    }
  }
  return true;
}
bool includer::parse_block_comment() noexcept {
  bool block_comment = true;
  while (block_comment) {
    tmp_c = i_file.peek();
    switch (tmp_c) {
      case '*': {
        if (!read_next("Read block comment after '*'", msg_type::warning)) {
          return false;
        }
        tmp_c = i_file.peek();
        if (tmp_c == '/') {
          if (!read_next("Read block comment without end line", msg_type::warning)) {
            return false;
          }
          block_comment = false;
        }
        break;
      }
      case '\n':
      case '\r': {  // end line comment
        parse_end_line(msg_type::none);
        if (endl_default) {
          return false;
        }
        break;
      }
      default: {
        if (!read_next(R"(Read block comment without "*/")", msg_type::warning)) {
          return false;
        }
        break;
      }
    }
  }
  return true;
}
bool includer::parse_string() noexcept {
  // TODO(kmosk): add check in R"()"
  //   std::string s = R"(This is "sample" string)";
  bool end_string = false;
  while (!end_string) {
    tmp_c = i_file.peek();
    switch (tmp_c) {
      case '\\': {  // exclude control symbols (\t, \n, \0 and other)
        if (!read_next("Read string after '\'")) {
          return false;
        }
        // TODO(kmosk): add check multi line string
        //   std::string s = "This is \
        //                    \"sample\" string";
        if (!read_next("Read string after control symbol")) {
          return false;
        }
        break;
      }
      case '"': {
        if (!read_next("Read end of string after '\"'")) {
          return false;
        }
        end_string = true;
        break;
      }
      default: {
        if (!read_next("Read end of string without '\"'")) {
          return false;
        }
        break;
      }
    }
  }
  return true;
}
bool includer::parse_directive() noexcept {
  tmp_c = i_file.peek();
  while (tmp_c == ' ') {  // TODO(kmsok): check only for space, add \t and ?other?
    if (!read_next("Read directive after '#'")) {
      return false;
    }
    tmp_c = i_file.peek();
  }
  // TODO(kmosk): check for block comment
  char buf[buf_len];
  i_file.read(buf, buf_len);
  if (i_file.eof()) {
    return false;
  }
  if (memcmp(include_string, buf, buf_len) == 0) {
    tmp_c = i_file.peek();
    while (tmp_c == ' ') {  // TODO(kmsok): check only for space, add \t and ?other?
      if (!read_next(R"(Read after "include")")) {
        return false;
      }
      tmp_c = i_file.peek();
    }
    tmp_c = i_file.peek();
    if (tmp_c == '/') {
      if (!read_next(R"(Read after "include /")")) {
        return false;
      }
      tmp_c = i_file.peek();
      if (tmp_c == '*') {
        if (!read_next(R"(Read after "include /*")")) {
          return false;
        }
        if (!parse_block_comment()) {
          return false;
        }
        tmp_c = i_file.peek();
        while (tmp_c == ' ') {  // TODO(kmsok): check only for space, add \t and ?other?
          if (!read_next(R"(Read after "include")")) {
            return false;
          }
          tmp_c = i_file.peek();
        }
      } else {
        message("Wrong syntax", R"(Expect '*' after "include /")");
        return false;
      }
    }
    // < and "
    tmp_c = i_file.peek();
    switch (tmp_c) {
      case '\"': {  // process "..."
        auto position = i_file.tellg();
        if (!read_next("Read after '\"'")) {
          return false;
        }
        if (!parse_include(true, position)) {
          return false;
        }
        break;
      }
      case '<': {  // process <...>
        auto position = i_file.tellg();
        if (!read_next("Read after '<'")) {
          return false;
        }
        if (!parse_include(false, position)) {
          return false;
        }
        break;
      }
      default: {
        // #if !defined(MBEDTLS_CONFIG_FILE)
        // #include "config.h"
        // #else
        // #include MBEDTLS_CONFIG_FILE
        // #endif
        return true;
        // message("Wrong syntax", "Expected '\"' or '<'");
        // return false;
      }
    }
  } else {
    for (unsigned long i(0); i < buf_len; ++i) {
      i_file.unget();
    }
  }
  return true;
}
bool includer::parse_include(bool exp, const ifstream::pos_type &position) noexcept {
  string include_path;
  bool end_include = false;
  while (!end_include) {
    tmp_c = i_file.peek();
    if (tmp_c == (exp ? '\"' : '>')) {
      read_next("Read last include symbol");
      end_include = true;
      break;
    }
    include_path += tmp_c;
    if (!read_next("Read include path")) {
      return false;
    }
  }
  // TODO(kmosk): include_path and exp is result
  include_t inc;
  inc.path = include_path;
  inc.position = position;
  inc.exp = exp;
  includes[i_fs_file->path()].push_back(inc);
  message((exp ? "\"\"" : "<>"), include_path, msg_type::found);
  return true;
}
bool includer::parse_end_line(msg_type type, bool def) noexcept {
  tmp_c = i_file.peek();
  switch (tmp_c) {
    case '\n': {  // line ends Unix style
      if (!read_next("Parse end line Unix", type)) {
        return false;
      }
      line++;
      break;
    }
    case '\r': {
      if (!read_next("Parse end line macOS", type)) {
        return false;
      }
      if (i_file.peek() == '\n') {  // line ends Windows style
        if (!read_next("Parse end line Windows", type)) {
          return false;
        }
      }  // else line ends macOS style
      line++;
      break;
    }
    default:
      if (def) {
        message("Parse end line error", "Wrong symbol");
        return endl_default = true;
      }
      if (!read_next("Parse end line default", type)) {
        return false;
      }
      break;
  }
  return true;
}
void includer::message(const string &info1, const string &info2, msg_type type) const noexcept {
  if (type != msg_type::none) {
    cout << msg_str.at(type) << " "
         << i_fs_file->path().string() << ':' << line
         << ":" << info1
         << ":" << info2 << endl;
  }
}
bool includer::read_next(const string &info, msg_type type) noexcept {
  i_file.get();
  if (i_file.eof()) {
    message("Unexpected end of file", info, type);
    return false;
  }
  return true;
}
