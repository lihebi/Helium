#include "ast.h"
#include "utils.h"

#include <gtest/gtest.h>


namespace ast {

  /*******************************
   ** traversal
   *******************************/

  /**
   * Next sibling on AST level
   */
  Node next_sibling(Node node) {
    Node n = node;
    while ((n = n.next_sibling())) {
      if (is_valid_ast(n)) return n;
    }
    return Node();
  }
  Node previous_sibling(Node node) {
    Node n =node;
    while ((n = n.previous_sibling())) {
      if (is_valid_ast(n)) return n;
    }
    return Node();
  }
  Node parent(Node node) {
    Node n = node;
    while ((n = n.parent())) {
      if (is_valid_ast(n)) return n;
    }
    return Node();
  }

  /**
   * valid ast includes: expr, decl, break, macro, for, while, if, function
   */

  bool is_valid_ast(const char* name) {
    if (strcmp(name, "expr_stmt") == 0
        || strcmp(name, "decl_stmt") == 0
        || strcmp(name, "break") == 0
        || strcmp(name, "macro") == 0
        || strcmp(name, "for") == 0
        || strcmp(name, "while") == 0
        || strcmp(name, "if") == 0
        || strcmp(name, "function") == 0
        ) return true;
    else return false;
  }


  bool is_valid_ast(Node node) {
    // return is_valid_ast(node.name());
    if (node.type() == pugi::node_element) return true;
    else return false;
  }


  /*******************************
   ** helium specific ast
   ** using helium_valid_ast
   *******************************/
  bool helium_is_valid_ast(Node n) {
    if (helium_valid_ast.find(kind(n)) != helium_valid_ast.end()) return true;
    else return false;
  }
  Node helium_next_sibling(Node node) {
    Node n = node;
    while ((n = n.next_sibling())) {
      // case is special. It is not parent of its block. It's sibling of the block.
      if (kind(n) == NK_Case) return Node();
      if (helium_is_valid_ast(n)) return n;
    }
    return Node();
  }
  Node helium_previous_sibling(Node node) {
    Node n = node;
    while ((n = n.previous_sibling())) {
      if (kind(n) == NK_Case) return Node();
      if (helium_is_valid_ast(n)) return n;
    }
    return Node();
  }
  Node helium_parent(Node node) {
    Node n = node;
    while ((n = n.parent())) {
      if (helium_is_valid_ast(n)) {
        // block directly under switch is not valid, because case: is invalid outside switch.
        if (kind(n) == NK_Block) {
          if (kind(n.parent()) == NK_Switch) {
            continue;
          }
        }
        return n;
      }
    }
    return Node();
  }


  /**
   * least upper bound of two nodes
   */
  pugi::xml_node
  lub(pugi::xml_node n1, pugi::xml_node n2) {
    if (n1.root() != n2.root()) return pugi::xml_node();
    pugi::xml_node root = n1.root();
    int num1=0, num2=0;
    pugi::xml_node n;
    n = n1;
    while (n!=root) {
      n = n.parent();
      num1++;
    }
    n = n2;
    while(n!=root) {
      n = n.parent();
      num2++;
    }
    if (num1 > num2) {
      // list 1 is longer
      while(num1-- != num2) {
        n1 = n1.parent();
      }
    } else {
      while(num2-- != num1) {
        n2 = n2.parent();
      }
    }
    // will end because the root is the same
    while (n1 != n2) {
      n1 = n1.parent();
      n2 = n2.parent();
    }
    return n1;
  }



  /**
   * Check if node is a sub node of any one of parent_nodes
   */
  bool contains(NodeList parent_nodes, Node node) {
    for (Node parent : parent_nodes) {
      if (contains(parent, node)) return true;
    }
    return false;
  }
  /**
   * Check if child is a sub node of parent
   */
  bool contains(Node parent, Node child) {
    if (lub(parent, child) == parent) return true;
    else return false;
  }

  /*******************************
   ** get_text family
   *******************************/
  
  std::string get_text(Node node) {
    std::string text;
    if (!node) return "";
    for (pugi::xml_node n : node.children()) {
      if (n.type() == pugi::node_element) {
        if (!node.attribute("helium-omit")) {
          // add text only if it is not in helium-omit
          text += get_text(n);
        } else if (strcmp(node.name(), "else") == 0 ||
                   strcmp(node.name(), "then") == 0 ||
                   strcmp(node.name(), "elseif") == 0 ||
                   strcmp(node.name(), "default") == 0
                   ) {
          // FIXME Why??????
          // For simplification of code.
          // I will add "helium-omit" attribute on the AST to mark deletion.
          // Those tag will be deleted.
          // But to make the syntax valid, I need to add some "{}"
          text += "{}";
        } else if (strcmp(node.name(), "case") == 0) {
          // FIXME why??????
          text += "{break;}";
        }
      } else {
        text += n.value();
      }
    }
    return text;
  }

  /**
   * get test of a list of nodes.
   * These nodes should be in the same level, i.e. nodes are siblings
   * Should get the pcdata after each node.
   * The nodes should be contineous? Not necessary.
   * FIXME deprecated! very buggy!
   */
  std::string get_text(NodeList nodes) {
    std::string result;
    for (Node n : nodes) {
      result += get_text(n);
      Node _n = n.next_sibling();
      while (_n.type() == pugi::node_pcdata) {
        result += _n.value();
        _n = _n.next_sibling();
      }
    }
    // but should not have the last, because that might be the end of function, }
    // if (nodes.empty()) return "";
    // if (nodes.size() == 1) return get_text(nodes[0]);
    // Node node = nodes[0];
    // Node node_end = *(nodes.end()-1);
    // Node n;
    // for (n=node;n!=node_end;n=n.next_sibling()) {
    //   assert(n && "n must exist, or the nodes are not in the same level");
    //   if (n.type() == pugi::node_element) {
    //     result += get_text(n);
    //   } else if (n.type() == pugi::node_pcdata) {
    //     result += n.value();
    //   } else {
    //     assert(false);
    //   }
    // }
    return result;
  }

  /**
   * Add line break between every node.
   */
  std::string get_text_ln(NodeList nodes) {
    std::string result;
    for (Node n : nodes) {
      result += get_text(n);
      result += '\n';
    }
    return result;
  }
  // std::string get_text_except(Node node, std::string tag) {
  //   if (!node) return "";
  //   std::string text;
  //   for (pugi::xml_node n : node.children()) {
  //     if (n.type() == pugi::node_element) {
  //       if (!node.attribute("helium-omit")) {
  //         if (strcmp(n.name(), tag.c_str()) != 0) {
  //           text += get_text_except(n, tag);
  //         }
  //       }
  //       // TODO this version does not use the trick for simplification,
  //       // so it doesnot work with simplification
  //     } else {
  //       text += n.value();
  //     }
  //   }
  //   return text;
  // }
  std::string get_text_except(Node node, NodeKind k) {
    return get_text_except(node, std::vector<NodeKind>(k));
  }
  std::string get_text_except(Node node, std::vector<NodeKind> kinds) {
    if (!node) return "";
    std::string result;
    for (Node n : node.children()) {
      if (n.type() == pugi::node_element) {
        for (NodeKind k : kinds) {
          if (kind(n) == k) {
            continue;
          }
        }
        result += get_text_except(n, kinds);
      } else {
        result += n.value();
      }
    }
    return result;
  }
  std::string get_text_except(NodeList nodes, NodeKind k) {
    std::string result;
    for (Node n : nodes) {
      result += get_text_except(n, k);
    }
    return result;
  }
  std::string get_text_except(NodeList nodes, std::vector<NodeKind> kinds) {
    std::string result;
    for (Node n : nodes) {
      result += get_text_except(n, kinds);
    }
    return result;
  }


  /**
   * True if node is inside a node of kind "kind"
   */
  bool in_node(Node node, NodeKind kind) {
    while ((node = parent(node))) {
      if (ast::kind(node) == kind) return true;
    }
    return false;
  }

  /*******************************
   ** --position related
   *******************************/

  /*
   * use depth-first-search for the first pos:line attribute
   * return -1 if no pos:line attr found
   */
  int
  get_node_line(pugi::xml_node node) {
    // check if pos:line is enabled on this xml
    pugi::xml_node root = node.root();
    if (!root.child("unit").attribute("xmlns:pos")) {
      // std::cerr<<"position is not enabled in srcml"<<std::endl;
      // exit(1);
      // should just return -1, it is by design, so that I can send some empty node in.
      return -1;
    }
    // the node itself has pos:line attr, just use it
    if (node.attribute("pos:line")) {
      return atoi(node.attribute("pos:line").value());
    } else {
      pugi::xml_node n = node.select_node(".//*[@pos:line]").node();
      if (n) {
        return atoi(n.attribute("pos:line").value());
      }
    }
    return -1;
  }

  /*
   * The last pos:line in the current node element
   * Useful to track the range of the current AST node.
   */
  int
  get_node_last_line(pugi::xml_node node) {
    pugi::xml_node root = node.root();
    if (!root.child("unit").attribute("xmlns:pos")) {
      std::cerr<<"position is not enabled in srcml"<<std::endl;
      exit(1);
      return -1;
    }
    pugi::xml_node n = node.select_node("(.//*[@pos:line])[last()]").node();
  
    // pugi::xpath_node_set nodes = node.select_nodes("//*[@pos:line]");
    // pugi::xml_node last_node = nodes[nodes.size()-1].node();
    // return atoi(last_node.attribute("pos:line").value());
  
    if (n) {
      return atoi(n.attribute("pos:line").value());
    } else if (node.attribute("pos:line")) {
      return atoi(node.attribute("pos:line").value());
    }
    return -1;
  }


  /*******************************
   ** Find nodes
   *******************************/

  /**
   * Find the children of node of kind "kind".
   * It doesn't need to be direct child.
   */
  NodeList find_nodes(Node node, NodeKind kind) {
    NodeList result;
    std::string tag = ".//";
    tag += kind_to_name(kind);
    pugi::xpath_node_set nodes = node.select_nodes(tag.c_str());
    for (auto it=nodes.begin();it!=nodes.end();++it) {
      result.push_back(it->node());
    }
    return result;
  }
  NodeList find_nodes(const Doc& doc, NodeKind kind) {
    Node root = doc.document_element();
    return find_nodes(root, kind);
  }
  NodeList find_nodes_from_root(Node node, NodeKind kind) {
    Node root = node.root();
    return find_nodes(root, kind);
  }

  /**
   * find nodes based on a list of kinds.
   */
  NodeList find_nodes(Node node, std::vector<NodeKind> kinds) {
    NodeList result;
    for (NodeKind k : kinds) {
      NodeList tmp = find_nodes(node, k);
      result.insert(result.end(), tmp.begin(), tmp.end());
    }
    return result;
  }
  NodeList find_nodes(const Doc& doc, std::vector<NodeKind> kinds) {
    Node root = doc.document_element();
    return find_nodes(root, kinds);
  }
  NodeList find_nodes_from_root(Node node, std::vector<NodeKind> kinds) {
    Node root = node.root();
    return find_nodes(root, kinds);
  }


  /*******************************
   ** based on line
   *******************************/

  /**
   * find node based on kind, but also *on* the line.
   */
  Node find_node_on_line(Node node, NodeKind k, int line_number) {
    NodeList nodes = find_nodes(node, k);
    for (Node n : nodes) {
      if (get_node_line(n) == line_number) {
        return n;
      }
    }
    return Node();
  }

  /**
   * find nodes whose kind is one of kinds, on that line.
   */
  Node find_node_on_line(Node node, std::vector<NodeKind> kinds, int line_number) {
    NodeList nodes = find_nodes(node, kinds);
    for (Node n : nodes) {
      if (get_node_line(n) == line_number) {
        return n;
      }
    }
    return Node();
  }
  /**
   * find one node on each one line in lines. The nodes must of kind `k`
   */
  NodeList find_nodes_on_lines(Node node, NodeKind k, std::vector<int> lines) {
    NodeList result;
    for (int l : lines) {
      result.push_back(find_node_on_line(node, k, l));
    }
    return result;
  }
  /**
   * find one node on each one line in lines. The node just need to satisfy one of the kinds.
   */
  NodeList find_nodes_on_lines(Node node, std::vector<NodeKind> kinds, std::vector<int> lines) {
    NodeList result;
    for (int l : lines) {
      result.push_back(find_node_on_line(node, kinds, l));
    }
    return result;
  }
  /**
   * The smallest tag enclosing line.
   */
  Node find_node_enclosing_line(Node node, NodeKind k, int line_number) {
    NodeList nodes = find_nodes(node, k);
    Node result;
    int cur_line = -1;
    for (Node n : nodes) {
      int first_line = get_node_line(node);
      int last_line = get_node_last_line(node);
      if (first_line <= line_number && last_line >= line_number) {
        // found a inner node, replace
        if (first_line > cur_line) {
          result = n;
          cur_line = first_line;
        }
      }
    }
    // may be an empty one
    return result;
  }
  /**
   * The outmost tag enclosing line
   */
  Node find_outer_node_enclosing_line(Node node, NodeKind k, int line_number) {
    NodeList nodes = find_nodes(node, k);
    Node result;
    int cur_line = line_number+1; // different from above function
    for (Node n : nodes) {
      int first_line = get_node_line(node);
      int last_line = get_node_last_line(node);
      if (first_line <= line_number && last_line >= line_number) {
        // found a inner node, replace
        if (first_line < cur_line) { // different from above function
          result = n;
          cur_line = first_line;
        }
      }
    }
    // may be an empty one
    return result;
  }

  /*******************************
   ** based on content (mainly comment)
   *******************************/
  /**
   * find the *first* node of kind k, under "node", whose text contains "s".
   */
  Node find_node_containing_str(Node node, NodeKind k, std::string s) {
    NodeList nodes = find_nodes(node, k);
    for (Node n : nodes) {
      std::string text = get_text(n);
      if (text.find(s) != std::string::npos) return n;
    }
    return Node();
  }
  /**
   * find *all* the nodes of kind k, under "node", whose text contains "s".
   */
  NodeList find_nodes_containing_str(Node node, NodeKind k, std::string s) {
    NodeList result;
    NodeList nodes = find_nodes(node, k);
    for (Node n : nodes) {
      std::string text = get_text(n);
      if (text.find(s) != std::string::npos) result.push_back(n);
    }
    return result;
  }
  Node find_node_containing_str(const Doc &doc, NodeKind k, std::string s) {
    return find_node_containing_str(doc.document_element(), k, s);
  }
  NodeList find_nodes_containing_str(const Doc &doc, NodeKind k, std::string s) {
    return find_nodes_containing_str(doc.document_element(), k, s);
  }



  /*******************************
   ** build doc from scratch
   *******************************/

  using namespace utils;
  /**
   * Get code as string of <tag_name> node in filename that encloses line_number
   * FIXME <tag> <tag> xxx </tag> </tag>, return which one?
   */
  std::string get_code_enclosing_line(const std::string& filename, int line_number, std::string tag_name) {
    // pugi::xml_document doc;
    // utils::file2xml(filename, doc);
    pugi::xml_document *doc = utils::file2xml(filename);
    pugi::xml_node root =doc->document_element();
    std::string query = ".//" + tag_name;
    pugi::xpath_node_set nodes = root.select_nodes(query.c_str());
    for (auto it=nodes.begin();it!=nodes.end();it++) {
      pugi::xml_node node = it->node();
      int first_line = get_node_line(node);
      int last_line = get_node_last_line(node);
      // FIXME the equal is necessary? Be precise.
      if (first_line <= line_number && last_line >= line_number) {
        return ast::get_text(node);
      }
    }
    return "";
  }

} // end of namespace ast
