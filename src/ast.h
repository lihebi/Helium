#ifndef __AST_H__
#define __AST_H__
#include <pugixml.hpp>

/**
 * Doc is a pointer, because it should be init and destroyed.
 */
typedef struct _ast_doc {
  pugi::xml_document doc;
} ast_doc;

/**
 * Node just maintain the xml_node itself, as well as the pointer to the ast_doc it corresponds to.
 */
typedef struct _ast_node {
  ast_doc *doc;
  pugi::xml_node node;
} ast_node;

ast_doc* ast_init_from_file(const std::string &filename);
ast_doc* ast_init_from_string(const std::string &code);
void ast_destroy(ast_doc* doc);

void ast_get_root();


void ast_get_previous_sibling();
void ast_get_next_sibling();

void ast_get_parent();

void ast_get_children();
void ast_get_first_child();

void ast_get_text();

class AST_Doc;

class AST_Node {
public:
  AST_Node();
  ~AST_Node();
  AST_Doc *GetDoc();
  AST_Node GetPreviousSibling();
  AST_Node GetNextSibling();
  AST_Node GetParent();
  AST_Node GetChildren();
  
private:
  AST_Doc *m_doc;
};

class AST_Doc {
public:
  AST_Doc();
  ~AST_Doc();
  AST_Node Root();
  void LoadFromFile(const std::string& filename);
  void LoadFromString(const std::string& code);
private:
};

#endif
