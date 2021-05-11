#include "includer.h"

int main(int argc, char *argv[]) {
  includer inc(argc, argv);
  if (!inc.run()) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}