#include "includer.h"
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;
using std::move;

const char *include_string = "include";

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
void includer::recursive_iteration() noexcept {
  for (auto &i : fs::recursive_directory_iterator(main_dir)) {
    auto ext = i.path().extension();
    if (ext == ".cpp" || ext == ".h") {
      line = 1;
      ifile.open(i.path());
      cout << endl << "passed" << i.path() << endl;
      while (!ifile.eof()) {
        tmp_c = ifile.peek();
        if (!continue_parse(i)) {
          break;
        }
      }
      ifile.close();
    }
  }
}
bool includer::continue_parse(const fs::directory_entry &i) noexcept {
  switch (tmp_c) {
    case '/': {  // exclude find in comment
      ifile.get();
      if (ifile.eof()) {
        cout << "error "
             << i.path().string() << ':' << line
             << ":Unexpected end of file"
             << ":Read after '/'" << endl;
        return false;
      }
      tmp_c = ifile.peek();
      switch (tmp_c) {
        case '/': {  // exclude find in line comment
          bool end_comment = false;
          while (!end_comment) {
            ifile.get();
            if (ifile.eof()) {  // file ends line comment
              return false;
            }
            tmp_c = ifile.peek();
            switch (tmp_c) {
              case '\n': {  // line ends Unix style
                ifile.get();
                line++;
                end_comment = true;
                break;
              }
              case '\r': {
                ifile.get();
                if (ifile.eof()) {  // file ends macOS style
                  return false;
                }
                if (ifile.peek() == '\n') {  // line ends Windows style
                  ifile.get();
                  line++;
                } else {  // line ends macOS style
                  line++;
                }
                end_comment = true;
                break;
              }
              case '\\': {  // multi-line comment
                ifile.get();
                if (ifile.eof()) {  // file ends '\'
                  return false;
                }
                tmp_c = ifile.peek();
                switch (tmp_c) {
                  case '\n': {  // line ends Unix style
                    ifile.get();
                    line++;
                    break;
                  }
                  case '\r': {
                    ifile.get();
                    if (ifile.eof()) {  // file ends macOS style
                      return false;
                    }
                    if (ifile.peek() == '\n') {  // line ends Windows style
                      ifile.get();
                      line++;
                    } else {  // line ends macOS style
                      line++;
                    }
                    break;
                  }
                  default:
                    break;
                }
              }
              default:
                break;
            }
          }
          break;
        }
        case '*': {  // exclude find in block comment
          // TODO: count lines
          bool block_comment = true;
          ifile.get();
          if (ifile.eof()) {
            cout << "error "
                 << i.path().string() << ':' << line
                 << ":Unexpected end of file"
                 << ":Read block comment" << endl;
            return false;
          }
          while (block_comment) {
            
          }
          break;
        }
        default: {
          cout << "error "
               << i.path().string() << ':' << line
               << ":Wrong syntax"
               << ":Expect '/' or '*' after '/'" << endl;
          return false;
        }
      }
      break;
    }
    case '\"': {  // exclude find in string
      // TODO(kmosk): add check in R"()" and multi line string
      bool end_string = false;
      while (!end_string) {
        ifile.get();
        if (ifile.eof()) {
          cout << "error "
               << i.path().string() << ':' << line
               << ":Unexpected end of file"
               << ":Read string" << endl;
          return false;
        }
        tmp_c = ifile.peek();
        switch (tmp_c) {
          case '\\': {
            ifile.get();
            if (ifile.eof()) {
              cout << "error "
                   << i.path().string() << ':' << line
                   << ":Unexpected end of file"
                   << ":Read string" << endl;
              return false;
            }
            ifile.get();
            if (ifile.eof()) {
              cout << "error "
                   << i.path().string() << ':' << line
                   << ":Unexpected end of file"
                   << ":Read string" << endl;
              return false;
            }
            break;
          }
          case '"': {
            ifile.get();
            if (ifile.eof()) {
              cout << "error "
                   << i.path().string() << ':' << line
                   << ":Unexpected end of file"
                   << ":Read string" << endl;
              return false;
            }
            end_string = true;
            break;
          }
        }
      }
      break;
    }
    case '#': {
      do {
        ifile.get();
        if (ifile.eof()) {
          cout << "error "
               << i.path().string() << ':' << line
               << ":Unexpected end of file"
               << ":Read after '#'" << endl;
          return false;
        }
        tmp_c = ifile.peek();
      } while (tmp_c == ' ');
      auto buf_len = strlen(include_string);
      char buf[buf_len];
      ifile.read(buf, buf_len);
      if (ifile.eof()) {
        cout << "error "
             << i.path().string() << ':' << line
             << ":Unexpected end of file"
             << ":Read after \"include\"" << endl;
        return false;
      }
      if (memcmp(include_string, buf, buf_len) == 0) {
        tmp_c = ifile.peek();
        while (tmp_c == ' ') {  // skip space
          ifile.get();
          if (ifile.eof()) {
            cout << "error "
                 << i.path().string() << ':' << line
                 << ":Unexpected end of file"
                 << ":Read after \"include\"" << endl;
            return false;
          }
          tmp_c = ifile.peek();
        }
        if (tmp_c == '/') {
          ifile.get();
          if (ifile.eof()) {
            cout << "error "
                 << i.path().string() << ':' << line
                 << ":Unexpected end of file"
                 << ":Read after '/'" << endl;
            return false;
          }
          tmp_c = ifile.peek();
          if (tmp_c == '*') {
            // TODO: count lines
            bool block_comment = true;
            while (block_comment) {
              ifile.get();
              if (ifile.eof()) {
                cout << "error "
                     << i.path().string() << ':' << line
                     << ":Unexpected end of file"
                     << ":Read block comment" << endl;
                return false;
              }
              tmp_c = ifile.peek();
              if (tmp_c == '*') {
                ifile.get();
                if (ifile.eof()) {
                  cout << "error "
                       << i.path().string() << ':' << line
                       << ":Unexpected end of file"
                       << ":Read block comment" << endl;
                  return false;
                }
                tmp_c = ifile.peek();
                if (tmp_c == '/') {
                  block_comment = false;
                }
              }
            }
          } else {
            cout << "error "
                 << i.path().string() << ':' << line
                 << ":Wrong syntax"
                 << ":Read after \"include \\\"" << endl;
            return false;
          }
        }
        tmp_c = ifile.peek();
        while (tmp_c == ' ') {  // skip space
          ifile.get();
          if (ifile.eof()) {
            cout << "error "
                 << i.path().string() << ':' << line
                 << ":Unexpected end of file"
                 << ":Read after \"include\"" << endl;
            return false;
          }
          tmp_c = ifile.peek();
        }
        switch (tmp_c) {
          case '\"': {  // process "..."
            string include_path;
            bool end_include = false;
            while (!end_include) {
              ifile.get();
              if (ifile.eof()) {
                cout << "error "
                     << i.path().string() << ':' << line
                     << ":Unexpected end of file"
                     << ":Read include" << endl;
                return false;
              }
              include_path += tmp_c;
              tmp_c = ifile.peek();
              if (tmp_c == '\"') {
                end_include = true;
              }
            }
            check_include(i, std::move(include_path));
            break;
          }
          case '<': {  // process <...>
            string include_path;
            bool end_include = false;
            while (!end_include) {
              ifile.get();
              if (ifile.eof()) {
                cout << "error "
                     << i.path().string() << ':' << line
                     << ":Unexpected end of file"
                     << ":Read include" << endl;
                return false;
              }
              include_path += tmp_c;
              tmp_c = ifile.peek();
              if (tmp_c == '\"') {
                end_include = true;
              }
            }
            check_include(i, std::move(include_path));
            break;
          }
          default: {
            cout << "error "
                 << i.path().string() << ':' << line
                 << ":Wrong syntax"
                 << ":Expected '\"' or '<'" << endl;
            return false;
          }
        }
      }
      break;
    }
    case '\n': {  // line ends Unix style
      ifile.get();
      line++;
      break;
    }
    case '\r': {
      ifile.get();
      if (ifile.eof()) {  // file ends macOS style
        return false;
      }
      if (ifile.peek() == '\n') {  // line ends Windows style
        ifile.get();
        line++;
      } else {  // line ends macOS style
        line++;
      }
      break;
    }
    default: {
      ifile.get();
      break;
    }
  }
  return true;
}
/**
 * @brief
 * @param i
 * @param inc - probably a string like [",<]path/to/include
 * @return
 */
bool includer::check_include(const fs::directory_entry &i,
                             string inc) noexcept {
  fs::path include_path;
  bool exp;
  switch (inc.at(0)) {
    case '\"': {
      exp = true;
      inc = inc.substr(1);
      break;
    }
    case '<': {
      exp = false;
      inc = inc.substr(1);
      break;
    }
    default:
      cout << "error "
           << i.path().string() << ':' << ifile.tellg()
           << R"(:Can not find '"' or '<')"
           << ':' << inc << endl;
      return false;
  }
  include_path = inc;
  cout << "found " << i.path().string() << ' '
       << (exp ? '"' : '<')
       << include_path.string()
       << (exp ? '"' : '>') << endl;
  return true;
}
