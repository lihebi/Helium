#include "builder.h"
#include <algorithm>
#include "utils.h"
#include "config.h"

Builder::Builder(Segment seg)
: m_seg(seg), m_success(false) {
}
Builder::~Builder() {}

void
Builder::writeMain() {
  utils::write_file(Config::Instance()->GetString("output-folder")+"/generate.c", m_main);
}

void Builder::writeSupport() {
  utils::write_file(
    Config::Instance()->GetString("output-folder")+"/support.h",
    m_support
  );
}

void
Builder::writeMakefile() {
  utils::write_file(
    Config::Instance()->GetString("output-folder")+"/Makefile",
    m_makefile
  );
}

void
Builder::Build() {
  m_main = m_seg.GetMain();
  m_support = m_seg.GetSupport();
  m_makefile = m_seg.GetMakefile();
  writeMain();
  writeSupport();
  writeMakefile();
}

void
Builder::Compile() {
  std::string clean_cmd = "make clean -C " + Config::Instance()->GetString("output-folder");
  std::string cmd = "make -C " + Config::Instance()->GetString("output-folder");
  // if (!Config::Instance()->WillShowCompileError()) {
  //   cmd += " 2>/dev/null";
  // }
  cmd += " 2>&1";
  utils::exec(clean_cmd.c_str(), NULL);
  int return_code;
  std::string error_msg = utils::exec(cmd.c_str(), &return_code);
  if (return_code != 0) {
    // if (Config::Instance()->WillInteractCompileError()) {
    //   std::cout<<"> Enter to continue ..."<<std::endl;
    //   getchar();
    // }
  } else {
    m_success = true;
  }
  // if (Config::Instance()->WillInteractCompile()) {
  //   std::cout<<"> Enter to continue ..."<<std::endl;
  //   getchar();
  // }
}

std::string Builder::GetExecutable() {
  return Config::Instance()->GetString("output-folder") + "/a.out";
}
