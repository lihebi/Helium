#ifndef SOURCE_MANAGER_H
#define SOURCE_MANAGER_H

#include <vector>
#include "helium/parser/AST.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <gtest/gtest.h>

#include "helium/parser/HeaderManager.h"

namespace fs = boost::filesystem;



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
  std::set<ASTNodeBase*> grammarPatch(std::set<ASTNodeBase*> sel);
  /**
   * Def use analysis
   * If some variable is used, include the node containing its declaration.
   */
  std::set<ASTNodeBase*> defUse(std::set<ASTNodeBase*> sel);

  /**
   * Generate program based on selection of nodes.
   * These nodes might be in different AST
   */
  std::string generateProgram(std::set<ASTNodeBase*>);

  /**
   * Generate main.h
   */
  std::string generateSupport(std::set<ASTNodeBase*>);

  /**
   * Generate main.c, main.h, Makefile, into a folder
   */
  void generate(std::set<ASTNodeBase*> sel, fs::path dir);

  /**
   * DEPRECATED
   * Get the UUID of a node.
   * This will be: filename_ID
   * If the node is an internal node of AST, the ID will be -1
   */
  std::string getTokenUUID(ASTNodeBase* node);
  fs::path getTokenFile(ASTNodeBase* node);
  int getTokenId(ASTNodeBase* node);


  /**
   * Selection
   */
  std::set<ASTNodeBase*> generateRandomSelection();

  /**
   * num: how many nodes to be selected.
   */
  std::set<ASTNodeBase*> genRandSel(int num);
  std::set<ASTNodeBase*> genRandSelFunc(int num);
  std::set<ASTNodeBase*> genRandSelSameFunc(int num);
  /**
   * load selection from file.
   * The format is:
   *
   * #file
   * line column
   * line column
   * TODO change such utility function to static to avoid mis-use
   */
  std::set<ASTNodeBase*> loadSelection(fs::path sel_file);
  std::set<ASTNodeBase*> loadJsonSelection(fs::path sel_file);
  /**
   * Dump selection into a file, and can be later load.
   * The dump information is more than the hand written one. It will contain the ID of the token.
   * The format will be:
   *
   * #file
   * line column ID
   * line column
   */
  void dumpSelection(std::set<ASTNodeBase*> selection, std::ostream &os);
  void dumpJsonSelection(std::set<ASTNodeBase*> selection, std::ostream &os);
  
  void analyzeDistribution(std::set<ASTNodeBase*> selection,
                           std::set<ASTNodeBase*> patch,
                           std::ostream &os);

  void dump(std::ostream &os);
  std::set<ASTNodeBase*> patchFunctionHeader(std::set<ASTNodeBase*> sel);

  void dumpDist(std::set<ASTNodeBase*> sel, std::ostream &os);
  std::set<ASTNodeBase*> filterLeaf(std::set<ASTNodeBase*> sel);
private:
  /**
   * Match a file in files and return the best match. Empty if no match.
   */
  fs::path matchFile(fs::path file);
  int getDistFile(std::set<ASTNodeBase*> sel);
  int getDistProc(std::set<ASTNodeBase*> sel);
  int getDistIf(std::set<ASTNodeBase*> sel);
  int getDistLoop(std::set<ASTNodeBase*> sel);
  int getDistSwitch(std::set<ASTNodeBase*> sel);
  
  /**
   * ASTs associcated with the nodes
   */
  std::set<ASTContext*> getASTByNodes(std::set<ASTNodeBase*> nodes);
  bool shouldPutIntoMain(std::set<ASTNodeBase*> sel);


  // this class holds all ASTs
  // it also determines the ID of ast nodes
  // each AST node should have an unique ID
  // std::vector<ASTContext*> ASTs;
  fs::path cppfolder;
  // std::vector<fs::path> files;

  std::map<fs::path, ASTContext*> File2ASTMap;
  std::map<ASTContext*, fs::path> AST2FileMap;

  // token visitor, always available because it is useful
  std::map<ASTContext*, TokenVisitor*> AST2TokenVisitorMap;
  std::map<ASTContext*, Distributor*> AST2DistributorMap;

  std::set<ASTNodeBase*> InputVarNodes;
  std::set<ASTNodeBase*> OutputVarNodes;


  std::map<ASTNodeBase*, SymbolTable> PersistTables;
  
  // std::vector<ASTNodeBase*> Nodes;
  // std::map<ASTNodeBase*,int> IDs;

  // std::set<ASTNodeBase*> selection;
};


#endif /* SOURCE_MANAGER_H */
