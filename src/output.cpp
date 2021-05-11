#include <cassert>
#include "output.h"

output *output::inst = nullptr;
output::output(std::ostream &log, std::ostream &warning, std::ostream &error)
    : log_(log), warning_(warning), error_(error) {}
void output::init(std::ostream &log, std::ostream &warning, std::ostream &error) {
  if (inst == nullptr) {
    inst = new output(log, warning, error);
  }
}
output &output::get() RELEASE_noexcept {
  assert(inst != nullptr);
  return *inst;
}
void output::destroy() {
  delete inst;
  inst = nullptr;
}
std::ostream &output::get_log() RELEASE_noexcept {
  assert(inst != nullptr);
  return log_;
}
std::ostream &output::get_warning() RELEASE_noexcept {
  assert(inst != nullptr);
  return warning_;
}
std::ostream &output::get_error() RELEASE_noexcept {
  assert(inst != nullptr);
  return error_;
}

