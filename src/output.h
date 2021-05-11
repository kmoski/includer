#ifndef OUTPUT_H_1A1753B0D511423A919E4FDE6C99551B
#define OUTPUT_H_1A1753B0D511423A919E4FDE6C99551B

#include <iostream>
#include "defines.h"

#define to_log     output::get().get_log()
#define to_warning output::get().get_warning()
#define to_error   output::get().get_error()

using std::endl;

class output {
  output(std::ostream &log, std::ostream &warning, std::ostream &error);
  static output *inst;
  std::ostream &log_;
  std::ostream &warning_;
  std::ostream &error_;
 public:
  output(output &) = delete;
  void operator=(const output &) = delete;
  static void init(std::ostream &log, std::ostream &warning, std::ostream &error);
  static output &get() RELEASE_noexcept;
  static void destroy();
  std::ostream &get_log() RELEASE_noexcept;
  std::ostream &get_warning() RELEASE_noexcept;
  std::ostream &get_error() RELEASE_noexcept;
};

#endif //OUTPUT_H_1A1753B0D511423A919E4FDE6C99551B
