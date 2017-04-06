#ifndef SOURCE_MANAGER_H
#define SOURCE_MANAGER_H

#include <vector>
#include "helium/parser/ast_v2.h"
#include "helium/utils/common.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <gtest/gtest.h>

namespace fs = boost::filesystem;


/**
 * Manager the headers on the system
 */
class HeaderManager {
public:
  static HeaderManager* Instance() {
    if (!instance) instance = new HeaderManager();
    return instance;
  }

  bool header_exists(const std::string header) {
    for (std::string s : Includes) {
      fs::path p(s + "/" + header);
      if (fs::exists(p)) return true;
    }
    return false;
  }

  void addConf(fs::path file) {
    std::map<std::string, std::string> headers = parseHeaderConf(file);
    for (auto m : headers) {
      if (header_exists(m.first)) {
        Headers.insert(m.first);
        if (!m.second.empty()) {
          Libs.insert(m.second);
        }
      } else {
        NonExistHeaders.insert(m.first);
      }
    }
  }

  /**
   * Discover headers used in the project dir, on current system, and
   * not in conf
   */
  std::set<std::string> discoverHeader(fs::path dir);
  /**
   * Discover headers used in project, but not on current system or
   * not in conf
   */
  std::set<std::string> checkHeader(fs::path dir);

  void dump(std::ostream &os) {
    os << "[HeaderManager] Headers: ";
    for (std::string s : Headers) {
      os << s << " ";
    }
    os << "\n";
    os << "[HeaderManager] Libs: ";
    for (std::string s : Libs) {
      os << s << " ";
    }
    os << "\n";
    os << "[HeaderManager] Includes: ";
    for (std::string s : Includes) {
      os << s << " ";
    }
    os << "\n";
    os << "[HeaderManager] Non-exist headers: ";
    for (std::string s : NonExistHeaders) {
      os << s << " ";
    }
    os << "\n";
  }
  /**
   * TODO Get headers
   */
  std::map<std::string, std::string> parseHeaderConf(fs::path file);

  std::set<std::string> getHeaders() {return Headers;}
  std::set<std::string> getLibs() {return Libs;}
  std::set<std::string> getIncludes() {return Includes;}
private:
  std::set<std::string> Includes = {
    "/usr/include/",
    "/usr/local/include/",
    "/usr/include/x86_64-linux-gnu/",
    "/usr/include/i386-linux-gnu/",
    "/usr/lib/gcc/x86_64-linux-gnu/6/include/",
    "/usr/lib/gcc/i386-linux-gnu/6/include/"
  };
  HeaderManager() {}
  ~HeaderManager() {}
  std::set<std::string> Headers;
  std::set<std::string> NonExistHeaders;
  std::set<std::string> Libs;
  static HeaderManager *instance;

};


/**
 * \ingroup parser
 */
class SourceManager {
public:
  /**
   * Precondition: p should be the cache/XXX/cpp folder
   */
  SourceManager(fs::path cppfolder);
  ~SourceManager() {}
  void dumpASTs();

  /**
   * Perform grammar patch based on this->selection.
   * Thus you need to call select first.
   */
  std::set<v2::ASTNodeBase*> grammarPatch(std::set<v2::ASTNodeBase*> sel);
  /**
   * Def use analysis
   * If some variable is used, include the node containing its declaration.
   */
  std::set<v2::ASTNodeBase*> defUse(std::set<v2::ASTNodeBase*> sel);

  /**
   * Generate program based on selection of nodes.
   * These nodes might be in different AST
   */
  std::string generateProgram(std::set<v2::ASTNodeBase*>);

  /**
   * Generate main.h
   */
  std::string generateSupport(std::set<v2::ASTNodeBase*>);

  /**
   * Generate main.c, main.h, Makefile, into a folder
   */
  void generate(std::set<v2::ASTNodeBase*> sel, fs::path dir);

  /**
   * DEPRECATED
   * Get the UUID of a node.
   * This will be: filename_ID
   * If the node is an internal node of AST, the ID will be -1
   */
  std::string getTokenUUID(v2::ASTNodeBase* node);
  fs::path getTokenFile(v2::ASTNodeBase* node);
  int getTokenId(v2::ASTNodeBase* node);


  /**
   * Selection
   */
  std::set<v2::ASTNodeBase*> generateRandomSelection();

  /**
   * num: how many nodes to be selected.
   */
  std::set<v2::ASTNodeBase*> genRandSel(int num);
  std::set<v2::ASTNodeBase*> genRandSelSameFile(int num);
  std::set<v2::ASTNodeBase*> genRandSelSameFunc(int num);
  /**
   * load selection from file.
   * The format is:
   *
   * #file
   * line column
   * line column
   * TODO change such utility function to static to avoid mis-use
   */
  std::set<v2::ASTNodeBase*> loadSelection(fs::path sel_file);
  /**
   * Dump selection into a file, and can be later load.
   * The dump information is more than the hand written one. It will contain the ID of the token.
   * The format will be:
   *
   * #file
   * line column ID
   * line column
   */
  void dumpSelection(std::set<v2::ASTNodeBase*> selection, std::ostream &os);
  void analyzeDistribution(std::set<v2::ASTNodeBase*> selection,
                           std::set<v2::ASTNodeBase*> patch,
                           std::ostream &os);

  void dump(std::ostream &os);
  std::set<v2::ASTNodeBase*> patchFunctionHeader(std::set<v2::ASTNodeBase*> sel);
private:
  /**
   * Match a file in files and return the best match. Empty if no match.
   */
  fs::path matchFile(fs::path file);
  int getDistFile(std::set<v2::ASTNodeBase*> sel);
  int getDistProc(std::set<v2::ASTNodeBase*> sel);
  int getDistIf(std::set<v2::ASTNodeBase*> sel);
  int getDistLoop(std::set<v2::ASTNodeBase*> sel);
  int getDistSwitch(std::set<v2::ASTNodeBase*> sel);
  
  /**
   * ASTs associcated with the nodes
   */
  std::set<v2::ASTContext*> getASTByNodes(std::set<v2::ASTNodeBase*> nodes);
  bool shouldPutIntoMain(std::set<v2::ASTNodeBase*> sel);


  // this class holds all ASTs
  // it also determines the ID of ast nodes
  // each AST node should have an unique ID
  // std::vector<v2::ASTContext*> ASTs;
  fs::path cppfolder;
  // std::vector<fs::path> files;

  std::map<fs::path, v2::ASTContext*> File2ASTMap;
  std::map<v2::ASTContext*, fs::path> AST2FileMap;

  // token visitor, always available because it is useful
  std::map<v2::ASTContext*, TokenVisitor*> AST2TokenVisitorMap;
  std::map<v2::ASTContext*, Distributor*> AST2DistributorMap;
  
  // std::vector<v2::ASTNodeBase*> Nodes;
  // std::map<v2::ASTNodeBase*,int> IDs;

  // std::set<v2::ASTNodeBase*> selection;
};


#endif /* SOURCE_MANAGER_H */
