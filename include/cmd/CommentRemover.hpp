#ifndef __COMMENT_REMOVER_HPP__
#define __COMMENT_REMOVER_HPP__

#include <string>
#include <vector>

class CommentRemover {
public:
  CommentRemover(const std::string &folder);
  virtual ~CommentRemover();

  void Run();
private:
  std::string m_folder;
  std::vector<std::string> m_files;
};

#endif
