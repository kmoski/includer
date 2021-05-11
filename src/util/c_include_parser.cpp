#include <cstring>
#include <cassert>
#include "c_include_parser.h"
#include "../output.h"

static const char *include_string = "include";
static const auto include_string_len = strlen(include_string);

bool c_include_parser::from_file(const fs::path *path_,
                                 std::vector<include_t> *includes_) RELEASE_noexcept {
  assert(path_ != nullptr);
  if (!fs::exists(*path_)) {
    message("File does not exist", path_->string(), msg_type::error);
    success = false;
    return success;
  }
  path = path_;
  includes = includes_;
  success = true;
  line = 1;
  file.open(*path);
  while (!file.eof()) {
    peek();
    if (!continue_parse()) {
      break;
    }
  }
  file.close();
  return success;
}
bool c_include_parser::continue_parse() RELEASE_noexcept {
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
      return read_next("Read after '\''") &&
             parse_symbol();
    }
    case '\n':
    case '\r': {
      // no need to get a character from the stream
      if (!parse_end_line()) {
        return false;
      }
      return true;
    }
    default: {
      return read_next("Continue parse", msg_type::none);
    }
  }
}
bool c_include_parser::parse_comment() RELEASE_noexcept {
  peek();
  switch (tmp_c) {
    case '/': {  // exclude find in line comment
      bool end_comment = false;
      if (!read_next(R"(Read after "//")")) {
        return false;
      }
      while (!end_comment) {
        peek();
        switch (tmp_c) {
          case '\n':
          case '\r': {  // end line comment
            if (!parse_end_line()) {
              return false;
            }
            end_comment = true;
            break;
          }
          case '\\': {  // multi-line comment???
            if (!read_next(R"(Read comment after "\")", msg_type::warning)) {
              return false;
            }
            // TODO(kmosk): multiline comment
            //  \ ;warning (0 - 32)
            //  \ ok;ok
            if (!parse_end_line(true)) {
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
      // message("??? syntax", "Expect '/' or '*' after '/'");
      return true;
    }
  }
  return true;
}
bool c_include_parser::parse_block_comment() RELEASE_noexcept {
  bool block_comment = true;
  while (block_comment) {
    peek();
    switch (tmp_c) {
      case '*': {
        if (!read_next("Read block comment after '*'", msg_type::warning)) {
          return false;
        }
        peek();
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
        if (!parse_end_line()) {
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
bool c_include_parser::parse_string() RELEASE_noexcept {
  // TODO(kmosk): add check in R"()"
  //   std::string s = R"(This is "sample" string)";
  bool end_string = false;
  while (!end_string) {
    peek();
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
bool c_include_parser::parse_directive() RELEASE_noexcept {
  if (!skip_space("Read directive after '#'")) {
    return false;
  }
  char buf[1024];
  std::streamsize len = include_string_len; // NOLINT(cppcoreguidelines-narrowing-conversions)
  file.read(buf, len);
  if (file.eof()) {
    return false;
  }
  if (memcmp(include_string, buf, len) == 0) {
    if (!parse_d_include()) {
      return false;
    }
  } else {
    // memset(buf, 0, len);
    for (unsigned long i(0); i < len; ++i) {
      file.unget();
    }
  }
  return true;
}
bool c_include_parser::parse_symbol() RELEASE_noexcept {
  peek();
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
  return true;
}
bool c_include_parser::parse_include(bool exp, const std::ifstream::pos_type &position) RELEASE_noexcept {
  include_t inc;
  std::string include_path;
  bool end_include = false;
  while (!end_include) {
    peek();
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
  inc.path = include_path;
  inc.position = position;
  inc.length = file.tellg() - position;
  inc.exp = exp;
  inc.line = line;
  includes->push_back(inc);
  return true;
}
bool c_include_parser::parse_end_line(bool def) RELEASE_noexcept {
  peek();
  switch (tmp_c) {
    case '\n': {  // line ends Unix style
      if (!read_next("Parse end line Unix", msg_type::none)) {
        return false;
      }
      line++;
      break;
    }
    case '\r': {
      if (!read_next("Parse end line macOS", msg_type::none)) {
        return false;
      }
      peek();
      if (tmp_c == '\n') {  // line ends Windows style
        if (!read_next("Parse end line Windows", msg_type::none)) {
          return false;
        }
      }  // else line ends macOS style
      line++;
      break;
    }
    default:
      if (def) {
        // message("Parser todo", "multiline comment", msg_type::warning);
        return true;
      } else {
        message("Parser error", "End line default", msg_type::error);
        success = false;
        return false;
      }
  }
  return true;
}
void c_include_parser::message_wrong_syntax(const std::string &msg, msg_type type) RELEASE_noexcept {
  message("Wrong syntax", msg, type);
}
void c_include_parser::message(const std::string &msg_info,
                               const std::string &msg,
                               msg_type type) RELEASE_noexcept {
  if (type != msg_type::none) {
    std::ostream *ss = &to_log;
    if (type == msg_type::warning) {
      ss = &to_warning;
    } else if (type == msg_type::error) {
      ss = &to_error;
    }
    *ss << "Include parser: " << msg_str.at(type) << ' '
         << path->string() << ':' << line
         << ": " << msg_info
         << ": " << msg << endl;
  }
}
void c_include_parser::peek() noexcept {
  tmp_c = file.peek(); // NOLINT(cppcoreguidelines-narrowing-conversions)
}
bool c_include_parser::read_next(const std::string &msg, msg_type type) RELEASE_noexcept {
  file.get();
  if (file.eof()) {
    message("Unexpected end of file", msg, type);
    if (type == msg_type::error) {
      success = false;
    }
    return false;
  }
  return true;
}
bool c_include_parser::skip_space(const std::string &msg) RELEASE_noexcept {
  peek();
  while (tmp_c == ' ') {  // TODO(kmsok): check only for space, add \t and ?other?
    if (!read_next(msg)) {
      return false;
    }
    peek();
  }
  return true;
}
bool c_include_parser::parse_d_include() RELEASE_noexcept {
  if (!skip_space(R"(Read after "include")")) {
    return false;
  }
  peek();
  if (tmp_c == '/') {
    if (!read_next(R"(Read after "include /")")) {
      return false;
    }
    peek();
    if (tmp_c == '*') {
      if (!read_next(R"(Read after "include /*")")) {
        return false;
      }
      if (!parse_block_comment()) {
        return false;
      }
      if (!skip_space(R"(Read after "include /*...*/")")) {
        return false;
      }
    } else {
      message_wrong_syntax(R"(Expect '*' after "include /")", msg_type::error);
      success = false;
      return false;
    }
  }
  // < and "
  peek();
  switch (tmp_c) {
    case '\"': {  // process "..."
      auto position = file.tellg();
      if (!read_next("Read after '\"'")) {
        return false;
      }
      if (!parse_include(true, position)) {
        return false;
      }
      break;
    }
    case '<': {  // process <...>
      auto position = file.tellg();
      if (!read_next("Read after '<'")) {
        return false;
      }
      if (!parse_include(false, position)) {
        return false;
      }
      break;
    }
    default: {
      // TODO(kmosk): define include
      //  #define myinc "myinc.h"
      //  #include myinc
      return true;
    }
  }
  return true;
}
