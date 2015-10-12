#include <Builder.hpp>
#include "util/FileUtil.hpp"
#include "util/ThreadUtil.hpp"
#include "Logger.hpp"

Builder::Builder(std::shared_ptr<SegmentProcessUnit> seg_unit)
: m_seg_unit(seg_unit), m_success(false) {
}
Builder::~Builder() {}

void
Builder::writeMain() {
  FileUtil::Write(
    Config::Instance()->GetOutputFolder()+"/generate.c",
    m_main
  );
}

void Builder::writeSupport() {
  FileUtil::Write(
    Config::Instance()->GetOutputFolder()+"/support.h",
    m_support
  );
}

void
Builder::writeMakefile() {
  FileUtil::Write(
    Config::Instance()->GetOutputFolder()+"/Makefile",
    m_makefile
  );
}

void
Builder::Build() {
  std::cout<<"[Builder][Build]"<<std::endl;
  m_main = m_seg_unit->GetMain();
  m_support = m_seg_unit->GetSupport();
  m_makefile = m_seg_unit->GetMakefile();
  writeMain();
  writeSupport();
  writeMakefile();
}

void
Builder::Compile() {
  std::cout<<"[Builder][Compile]"<<std::endl;
  std::string clean_cmd = "make clean -C " + Config::Instance()->GetOutputFolder();
  std::string cmd = "make -C " + Config::Instance()->GetOutputFolder();
  std::cout<<"[Builder][Compile] clean"<<std::endl;
  if (!Config::Instance()->WillShowCompileError()) {
    cmd += " 2>/dev/null";
  }
  ThreadUtil::Exec(clean_cmd);
  std::cout<<"[Builder][Compile] make"<<std::endl;
  std::cout<<cmd<<std::endl;
  int return_code = ThreadUtil::ExecExit(cmd);
  if (return_code != 0) {
    std::cout<<"[Builder][Compile]"<<"\033[31m"<<"compile error"<<"\033[0m"<<std::endl;
    Logger::Instance()->Logln("/tmp/helium_buildrate.txt", "compile error");
    if (Config::Instance()->WillInteractCompileError()) {
      std::cout<<"> Enter to continue ..."<<std::endl;
      getchar();
    }
  } else {
    std::cout<<"[Builder][Compile]"<<"\033[32m"<<"compile success"<<"\033[0m"<<std::endl;
    Logger::Instance()->Logln("/tmp/helium_buildrate.txt", "compile success");
    m_success = true;
  }
  if (Config::Instance()->WillInteractCompile()) {
    std::cout<<"> Enter to continue ..."<<std::endl;
    getchar();
  }
}

std::string Builder::GetExecutable() {
  return Config::Instance()->GetOutputFolder() + "/a.out";
}
