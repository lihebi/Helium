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

#include <spdlog/spdlog.h>
#include "resolver/Ctags.hpp"

#include "util/FileUtil.hpp"
#include "resolver/SystemResolver.hpp"
#include "resolver/HeaderSorter.hpp"
#include "cmd/CondComp.hpp"

namespace fs = boost::filesystem;

int main(int argc, char* argv[]) {
  const char *helium_home_env = std::getenv("HELIUM_HOME");
  spdlog::stdout_logger_mt("console");
  if(!helium_home_env) {
    std::cout<<"Please set env variable HELIUM_HOME"<<std::endl;
    return 1;
  }
  fs::path helium_home(helium_home_env);
  ArgParser *arg_parser = new ArgParser(argc, argv);
  std::string folder = arg_parser->GetString("folder");
  fs::path project_folder(folder);
  std::string tagfile = (project_folder / "tags").string();
  // utils
  if (arg_parser->HasCmdUtils()) {
    if (arg_parser->Has("split")) {
      Splitter(folder).Run();
    } else if (arg_parser->Has("remove-comment")) {
      CommentRemover(folder).Run();
    } else if (arg_parser->Has("ctags")) {
      std::string cmd = "ctags -f ";
      cmd += tagfile;
      cmd += " --languages=c,c++ -n --c-kinds=+x --exclude=heium_out -R ";
      cmd += folder;
      std::cout<<cmd<<std::endl;
      std::system(cmd.c_str());
    } else if (arg_parser->Has("cond-comp")) {
      CondComp(folder).Run();
    }
  } else {
    // main
    fs::path config_file;
    if (arg_parser->Has("build-rate")) {
      config_file =  helium_home / "buildrate.xml";
    } else if (arg_parser->Has("equivalence")) {
      config_file = helium_home / "equivalence.xml";
    } else if (arg_parser->Has("change")) {
      config_file = helium_home / "change.xml";
    } else if (arg_parser->Has("config")) {
      config_file = arg_parser->GetString("config");
    } else {
      config_file = helium_home / "helium.xml";
    }
    Config *config = Config::Instance();
    config->Load(config_file.string());
    Ctags::Instance()->Load(tagfile);
    SystemResolver::Instance()->Load((helium_home / "systype.tags").string());
    HeaderSorter::Instance()->Load(project_folder.string());
    std::string output_folder = config->GetOutputFolder();
    FileUtil::RemoveFolder(output_folder);
    FileUtil::CreateFolder(output_folder);
    new Helium(folder);
  }
  return 0;
}
