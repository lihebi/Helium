#include <iostream>
#include <cstdlib>

#include "workflow/helium.h"
#include "config/config.h"

int main(int argc, char* argv[]) {
  utils::seed_rand();
  Helium *helium = new Helium(argc, argv);
  helium->Run();
  return 0;
}
