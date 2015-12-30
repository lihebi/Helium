#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>

#include <cstdlib>

#include "Helium.hpp"
#include "Config.hpp"
#include "cmd/CommentRemover.hpp"
#include "cmd/Splitter.hpp"
#include "ArgParser.hpp"

#include "resolver/Ctags.hpp"

#include "util/FileUtil.hpp"
#include "resolver/SystemResolver.hpp"
#include "resolver/HeaderSorter.hpp"
#include "cmd/CondComp.hpp"



int main(int argc, char* argv[]) {
  Helium *helium = new Helium(argc, argv);
  helium->Run();
  return 0;
}
