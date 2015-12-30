#include <Builder.hpp>
#include <algorithm>
#include "util/FileUtil.hpp"
#include "util/ThreadUtil.hpp"

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
  m_main = m_seg_unit->GetMain();
  m_support = m_seg_unit->GetSupport();
  m_makefile = m_seg_unit->GetMakefile();
  writeMain();
  writeSupport();
  writeMakefile();
}

void
Builder::Compile() {
  std::string clean_cmd = "make clean -C " + Config::Instance()->GetOutputFolder();
  std::string cmd = "make -C " + Config::Instance()->GetOutputFolder();
  // if (!Config::Instance()->WillShowCompileError()) {
  //   cmd += " 2>/dev/null";
  // }
  cmd += " 2>&1";
  ThreadUtil::Exec(clean_cmd.c_str(), NULL);
  int return_code;
  std::string error_msg = ThreadUtil::Exec(cmd.c_str(), &return_code);
  if (return_code != 0) {
    if (Config::Instance()->WillBuildSaveIncompilable()) {
      std::string to_folder = Config::Instance()->GetTmpFolder() + "/generated_code/incompilable";
      std::string folder = m_seg_unit->GetFilename()
      + "-" + std::to_string(m_seg_unit->GetLineNumber()) + "-" + __DATE__ + "-" + __TIME__;
      folder.erase(std::remove(folder.begin(), folder.end(), ' '), folder.end());
      std::replace(folder.begin(), folder.end(), '/', '-');
      FileUtil::CreateFolder(to_folder);
      std::string cmd = "cp -r " + Config::Instance()->GetOutputFolder()
      + " " + to_folder + "/" + folder;
      ThreadUtil::Exec(cmd.c_str(), NULL);
    }
    if (Config::Instance()->WillInteractCompileError()) {
      std::cout<<"> Enter to continue ..."<<std::endl;
      getchar();
    }
  } else {
    m_success = true;
    if (Config::Instance()->WillBuildSaveCompilable()) {
      std::string to_folder = Config::Instance()->GetTmpFolder() + "/generated_code/compilable";
      std::string folder = m_seg_unit->GetFilename()
      + "-" + std::to_string(m_seg_unit->GetLineNumber()) + "-" + __DATE__ + "-" + __TIME__;
      folder.erase(std::remove(folder.begin(), folder.end(), ' '), folder.end());
      std::replace(folder.begin(), folder.end(), '/', '-');
      FileUtil::CreateFolder(to_folder);
      std::string cmd = "cp -r " + Config::Instance()->GetOutputFolder()
      + " " + to_folder + "/" + folder;
      ThreadUtil::Exec(cmd.c_str(), NULL);
    }
  }
  if (Config::Instance()->WillInteractCompile()) {
    std::cout<<"> Enter to continue ..."<<std::endl;
    getchar();
  }
}

std::string Builder::GetExecutable() {
  return Config::Instance()->GetOutputFolder() + "/a.out";
}
