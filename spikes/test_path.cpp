#include <iostream>
#include <boost/filesystem.hpp>

int main(int argc, char** argv) {
  boost::filesystem::path dir("user/hello/world");
  boost::filesystem::path file("foo.txt");
  boost::filesystem::path full = dir / file;
  std::cout<<full<<std::endl;
  // native
  // std::cout<<full.native()<<std::endl;
  // std::cout<<full.c_str()<<std::endl;
  // @return string
  std::cout<<full.string()<<std::endl;
  // std::cout<<full.generic_string()<<std::endl;
  // decomposition
  std::cout<<full.root_name()<<std::endl;
  std::cout<<full.root_directory()<<std::endl;
  std::cout<<full.relative_path()<<std::endl;
  std::cout<<full.parent_path()<<std::endl;
  // @return type
  std::cout<<full.filename()<<std::endl; // foo.txt
  std::cout<<full.stem()<<std::endl; // foo
  std::cout<<full.extension()<<std::endl; // .txt
  // query
  full.empty();
  full.is_absolute();
  full.is_complete();
  full.is_relative();
  full.has_extension();
  full.has_filename();
  full.has_parent_path();
  full.has_relative_path();
  full.has_root_directory();
  full.has_root_name();
  full.has_root_path();
}
