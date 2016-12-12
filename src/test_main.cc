#include <gtest/gtest.h>
#include "helium_options.h"
#include "resolver/snippet_db.h"
#include "resolver/system_resolver.h"
#include "utils/utils.h"

int main(int argc, char** argv) {
  utils::seed_rand();

  const char *home = getenv("HOME");
  if (!home) {
    std::cerr << "EE: HOME env is not set." << "\n";
    exit(1);
  }
  HeliumOptions::Instance()->ParseConfigFile("~/.heliumrc");
  std::string helium_home = HeliumOptions::Instance()->GetString("helium-home");
  helium_home = utils::escape_tide(helium_home);
  SystemResolver::Instance()->Load(helium_home + "/systype.tags");

  
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
