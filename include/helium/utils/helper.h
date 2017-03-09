#ifndef HELPER_H
#define HELPER_H

#include <string>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;


int get_true_linum(std::string filename, int linum);

class Gcov {
public:
Gcov(fs::path folder, fs::path src_file);
~Gcov() {}
std::string GetStmtCoverage() {
return m_stmt_cov;
}
std::string GetBranchCoverage() {
return m_branch_cov;
}
private:
fs::path m_folder;
fs::path m_src_file;
std::string m_stmt_cov;
std::string m_branch_cov;
};

#endif /* HELPER_H */
