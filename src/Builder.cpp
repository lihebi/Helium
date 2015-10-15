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
  Logger::Instance()->LogTrace("[Builder][Build]\n");
  m_main = m_seg_unit->GetMain();
  m_support = m_seg_unit->GetSupport();
  m_makefile = m_seg_unit->GetMakefile();
  writeMain();
  writeSupport();
  writeMakefile();
}

void
Builder::Compile() {
  Logger::Instance()->LogTrace("[Builder][Compile]\n");
  std::string clean_cmd = "make clean -C " + Config::Instance()->GetOutputFolder();
  std::string cmd = "make -C " + Config::Instance()->GetOutputFolder();
  Logger::Instance()->LogTrace("[Builder][Compile] clean\n");
  // if (!Config::Instance()->WillShowCompileError()) {
  //   cmd += " 2>/dev/null";
  // }
  cmd += " 2>&1";
  ThreadUtil::Exec(clean_cmd.c_str(), NULL);
  Logger::Instance()->LogTrace("[Builder][Compile] make\n");
  int return_code;
  std::string error_msg = ThreadUtil::Exec(cmd.c_str(), &return_code);
  if (return_code != 0) {
    Logger::Instance()->LogTrace("[Builder][Compile] compile error\n");
    Logger::Instance()->LogRate("compile error\n");
    Logger::Instance()->LogCompile(error_msg);
    if (Config::Instance()->WillInteractCompileError()) {
      std::cout<<"> Enter to continue ..."<<std::endl;
      getchar();
    }
  } else {
    Logger::Instance()->LogTrace("[Builder][Compile] compile success\n");
    Logger::Instance()->LogRate("compile success\n");
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
