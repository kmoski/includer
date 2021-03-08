#include "includer.h"

int main(int argc, char *argv[]) {
  includer *inc;
  try {
    inc = new includer(argc, argv);
  } catch (...) {
    return -1;
  }
  inc->recursive_iteration();
  return 0;
}