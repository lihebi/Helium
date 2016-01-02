#ifndef __HELIUM_H__
#define __HELIUM_H__

class Helium {
public:
  Helium(int argc, char** argv);
  virtual ~Helium();
  void Run();
private:
  std::string m_folder;
  std::vector<std::string> m_files;
};

#endif
