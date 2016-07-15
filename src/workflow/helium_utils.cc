#include "helium_utils.h"
#include <iostream>
/**
 * read env variable HELIUM_HOME
 * @return std::string home folder without trailing '/'
 */
std::string
load_helium_home() {
  /* read HELIUM_HOME env variable */
  const char *helium_home_env = std::getenv("HELIUM_HOME");
  if(!helium_home_env) {
    std::cerr<<"Please set env variable HELIUM_HOME"<<std::endl;
    exit(1);
  }
  std::string helium_home = helium_home_env;
  while (helium_home.back() == '/') helium_home.pop_back();
  return helium_home;
}

