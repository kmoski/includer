#include "includer.h"
#include <iostream>
#include <cstring>

using std::cout;
using std::endl;
using std::move;

const char *include_string = "include";

includer::includer(const int &argc, char **argv) {
  if (argc < 3) {
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
  for (unsigned i{2}; i < argc; ++i) {
    include_dirs.emplace_back(argv[i]);
    if (!fs::exists(include_dirs.back()) ||
        !fs::is_directory(include_dirs.back())) {
      cout << "Incorrect include directory:" << endl;
      cout << include_dirs.back() << endl;
      throw;
    }
  }
}
void includer::recursive_iteration() noexcept {
  for (auto &i : fs::recursive_directory_iterator(main_dir)) {
    auto ext = i.path().extension();
    if (ext == ".cpp" || ext == ".h") {
      line = 1;
      ifile.open(i.path());
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
          // TODO(kmosk): exclude find in block comment
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
      // TODO(kmosk): exclude find in string
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
            // TODO(kmosk): process block comment
          } else {
            // TODO(kmosk): error
          }
        }
        // TODO(kmosk): process <> or ""
      }



      // if (numb_pos != string::npos) {  // tmp contain '#'
      //   if (tmp.length() == 1) {  // read only one '#'
      //     if (!(ifile >> tmp)) {  // read next word
      //       cout << "error "
      //            << i.path().string() << ':' << ifile.tellg()
      //            << ":Unexpected end of file"
      //            << ":Read after #" << endl;
      //       break;
      //     }
      //     if (!find_include(i, "include")) {
      //       break;
      //     }
      //   } else {
      //     if (!find_include(i, "#include")) {
      //       break;
      //     }
      //   }
      // }
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
    default: break;
  }
  return true;
}
bool includer::find_include(const fs::directory_entry &i,
                            const char *sub) noexcept {
  auto inc_pos = tmp.find(sub);
  if (inc_pos != string::npos) {  // tmp contain "include" or "#include"
    if (tmp == sub) {  // tmp equal "include" or "#include"
      if (!(ifile >> tmp)) {  // read after include
        cout << "error "
             << i.path().string() << ':' << ifile.tellg()
             << ":Unexpected end of file"
             << ":Read after \"[#]include\"" << endl;
        return false;
      }
      if (!check_include(i, move(tmp))) {
        return false;
      }
    } else {  // tmp contain sub, but contain anything else
      if (!check_include(i, move(tmp.substr(strlen(sub))))) {
        return false;
      }
    }
  }
  return true;
}
bool includer::check_include(const fs::directory_entry &i,
                             string inc) noexcept {
  fs::path include_path;
  bool exp;
  unsigned long pos;
  switch (inc.at(0)) {
    case '\"': {
      exp = true;
      inc = inc.substr(1);
      pos = inc.find('\"');
      break;
    }
    case '<': {
      exp = false;
      inc = inc.substr(1);
      pos = inc.find('>');
      break;
    }
    default:
      cout << "error "
           << i.path().string() << ':' << ifile.tellg()
           << R"(:Can not find '"' or '<')"
           << ':' << tmp << endl;
      return false;
  }
  if (pos == string::npos) {
    cout << "error "
         << i.path().string() << ":" << ifile.tellg()
         << R"(:Can not find '"' or '>')"
         << ':' << tmp << endl;  // TODO(kmosk): space
    return false;
  }
  include_path = inc.substr(0, pos);
  cout << "found " << i.path().string() << ' '
       << (exp ? '"' : '<')
       << include_path.string()
       << (exp ? '"' : '>') << endl;
  // TODO(kmosk): check
  return true;
}
