#include <gtest/gtest.h>
#include "helium/utils/helium_options.h"
#include "helium/resolver/snippet_db.h"
#include "helium/resolver/system_resolver.h"
#include "helium/utils/utils.h"

int main(int argc, char** argv) {
  utils::seed_rand();

  const char *home = getenv("HOME");
  if (!home) {
    std::cerr << "EE: HOME env is not set." << "\n";
    exit(1);
  }
  HeliumOptions::Instance()->ParseConfigFile("~/.heliumrc");
  // std::string helium_home = HeliumOptions::Instance()->GetString("helium-home");
  // helium_home = utils::escape_tide(helium_home);
  // SystemResolver::Instance()->Load(helium_home + "/systype.tags");

  fs::path user_home(getenv("HOME"));
  fs::path helium_home = user_home / ".helium.d";
  if (!fs::exists(helium_home)) {
    fs::create_directory(helium_home);
  }
  fs::path systag = helium_home / "systype.tags";
  if (!fs::exists(systag)) {
    if (HeliumOptions::Instance()->Has("setup")) {
      // run ctags to create tags
      std::cout << "Creating ~/systype.tags" << "\n";
      std::string cmd = "ctags -f " + systag.string() +
        " --exclude=boost"
        " --exclude=llvm"
        " --exclude=c++"
        " --exclude=linux"
        " --exclude=xcb"
        " --exclude=X11"
        " --exclude=openssl"
        " --exclude=xorg"
        " -R /usr/include/ /usr/local/include";
      std::cout << "Running " << cmd << "\n";
      utils::exec(cmd.c_str());
      std::cout << "Done" << "\n";
      exit(0);
    } else {
      std::cout << "No systype.tags found. Run helium --setup first." << "\n";
      exit(1);
    }
  }
  SystemResolver::Instance()->Load(systag.string());
  
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
