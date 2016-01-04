#include <iostream>
#include <cstdlib>

#include "helium.h"
#include "config.h"

int main(int argc, char* argv[]) {
  Helium *helium = new Helium(argc, argv);
  helium->Run();
  return 0;
}
