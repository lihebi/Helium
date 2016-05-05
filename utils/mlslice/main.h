#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <pugixml.hpp>
#include "../../src/ast_node.h"

// Pointer, Reference, Class, Struct, Primitive
// typedef enum {
//   MLST_Pointer,
//   MLST_Reference,
//   MLST_Array,
//   MLST_Struct,
//   MLST_Primitive
// } MLSliceTypeKind;

// typedef struct _MLSliceType {
//   unsigned int is_pointer : 1;
//   unsigned int is_reference : 1;
//   unsigned int is_array : 1;
//   unsigned int is_struct : 1;
//   unsigned int is_primitive : 1;
// } MLSliceType;

class MLSliceType {
public:
  unsigned int GetValue() const {return value;}
  void SetPointer()
  {value |= 1;}
  void SetReference()
  {value |= 1<<1;}
  void SetArray()
  {value |= 1<<3;}
  void SetStruct()
  {value |= 1<<3;}
  void SetPrimitive()
  {value |= 1<<4;}
  void clear() {value=0;}
  void Set(unsigned int v) {value=v;}
private:
  unsigned int value = 0;
};


class SlicingCriteria {
public:
  SlicingCriteria(std::string abs, int l);
  ~SlicingCriteria() {
    if (m_doc) {delete m_doc;}
    if (m_ast) delete m_ast;
  }
  std::string GetAbsFilename() const {
    return m_abs_filename;
  }
  int GetLinum() const {return m_linum;}
  pugi::xml_document *GetDoc() {return m_doc;}
  pugi::xml_node GetXMLRoot() {return m_root;}
  pugi::xml_node GetXMLFuncNode() {return m_func;}

  ast::AST *GetAST() {return m_ast;}
  
  int GetFuncBeginLinum();
  int GetFuncEndLinum();
  unsigned int LookUpVariable(std::string name) {
    if (m_sym_tbl.count(name) == 0) return 0;
    return m_sym_tbl[name];
  }
  ast::ASTNode* GetASTNode() {return m_astnode;}
private:
  void resolveVars();
  std::string m_abs_filename;
  int m_linum = 0;
  std::string m_line;
  pugi::xml_document *m_doc = NULL;
  pugi::xml_node m_func;
  pugi::xml_node m_root;
  ast::ASTNode *m_astnode = NULL;
  ast::AST *m_ast = NULL;

  std::map<std::string, unsigned int> m_sym_tbl;
  
};

// TODO features
// f1 variable name
// f2 variable type
// f3: distance
// f4: AST level
// f5: Transformation Count
class FeatureRecord {
public:
  static std::string header() {
    return "filename, linum,"
      "f1:same_var, f2:same_var_type, f3:distance, f4:AST_Distance,"// f5:trans_count,"
      "f5:ast_dis_t_lca, f6:ast_dis_poi_lca, f7:ast_lca_lvl,"
      "f8:use_global, "
      "in_slice_or_not";
  }
  std::string dump() {
    // <ID>, <filename>, <line number> <f1>, <f2>, <f3>, true(false)
    // <line number> <f1>, <f2>, <f3>, true(false)
    std::string ret =
      filename + ", "
      + std::to_string(linum) + ", "
      + std::to_string(var_name_used_in_POI) + ", "
      + std::to_string(var_type) + ", "
      + std::to_string(distance) + ", "
      + std::to_string(AST_distance) + ", "
      // f5-f7
      + std::to_string(ast_dis_t_lca) + ", "
      + std::to_string(ast_dis_poi_lca) + ", "
      + std::to_string(ast_lca_lvl) + ", "
      + (use_global?"1":"0") + ", "
      // + std::to_string(trans_count) + ", "
      + (in_slice?"1":"0");
    return ret;
  }
  std::string filename;
  int linum = 0;
  bool var_name_used_in_POI = false;
  unsigned int var_type = 0;
  int distance = 0; // this can be negative
  int AST_distance = 0; // the path from T to POI by traversing AST
  int ast_dis_t_lca = 0;
  int ast_dis_poi_lca = 0;
  int ast_lca_lvl=0;
  bool use_global = false;
  int trans_count = 0; // TODO
  bool in_slice=false;
};

#endif /* MAIN_H */
