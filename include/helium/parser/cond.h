#ifndef COND_H
#define COND_H

#include "helium/utils/common.h"
#include "xmlnode.h"
#include "xmlnode_helper.h"
#include "xml_doc_reader.h"

/**
 * Condition
 */


class Cond {
public:
  Cond(std::string text) {
    m_text = text;
    m_left = NULL;
    m_right = NULL;
  }
  Cond(Cond *left, Cond *right, std::string op) {
    m_left = left;
    m_right = right;
    m_op = op;
  }
  ~Cond() {
    delete m_left;
    delete m_right;
  }

  std::string GetOp() {return m_op;}

private:
  std::string m_op;
  Cond *m_left;
  Cond *m_right;
  std::string m_text;
};


typedef enum _OpKind {
  OK_Greater,
  OK_Less,
  OK_Equal,
  OK_NotEqual,
  OK_GreaterOrEqual,
  OK_LessOrEqual
} OpKind;


OpKind Str2OpKind(std::string text);
std::string OpKind2Str(OpKind op);

/**
 * Only consider <expr> <operator> <expr>
 * And this also don't use for a->b
 */
class SimpleCond {
public:
  SimpleCond(std::string left, OpKind op, std::string right)
    : m_left(left), m_op(op), m_right(right) {}
  /**
   * Get SMT string
   */
  std::string GetSMT() {
    // TODO
    return "";
  }
  std::string GetLeft() {return m_left;}
  std::string GetRight() {return m_right;}
  std::string GetOp() {return OpKind2Str(m_op);}
private:
  std::string m_left;
  OpKind m_op;
  std::string m_right;
};

class SimpleCondFactory {
public:
  static SimpleCond *Create(std::string text) {
    SimpleCond *ret = NULL;
    XMLDoc *doc = XMLDocReader::CreateDocFromString(text);
    XMLNode expr = find_first_node_bfs(doc->document_element(), "expr");
    std::vector<XMLNode> nodes;
    for (XMLNode node : expr.children()) {
      nodes.push_back(node);
    }
    if (nodes.size() == 1) {
      ret = new SimpleCond(get_text(nodes[0]), OK_NotEqual, "0");
    } else if (nodes.size() == 3) {
      if (std::string("operator") == nodes[1].name()) {
        std::string left = get_text(nodes[0]);
        if (left == "NULL") left = "0";
        std::string right = get_text(nodes[2]);
        if (right == "NULL") right = "0";
        std::string op = get_text(nodes[1]);
        try {
          ret = new SimpleCond(left, Str2OpKind(op), right);
        } catch (HeliumException) {
          if (ret) delete ret;
          ret = NULL;
        }
      }
    }
    return ret;
  }
};

class CondFactory {
public:
  static Cond *Create(std::string text) {
    XMLDoc *doc = XMLDocReader::CreateDocFromString(text);
    Cond *ret=NULL;
    XMLNode expr = find_first_node_bfs(doc->document_element(), "expr");
    std::vector<XMLNode> nodes;
    for (XMLNode node : expr.children()) {
      nodes.push_back(node);
    }
    if (nodes.size() == 1) {
      // name!=0
      ret = new Cond(new Cond(get_text(nodes[0])),
                        new Cond("0"), "!=");
    } else if (nodes.size() == 3) {
      if (std::string("operator") == nodes[1].name()) {
        std::string left = get_text(nodes[0]);
        if (left == "NULL") left = "0";
        std::string right = get_text(nodes[2]);
        if (right == "NULL") right = "0";
        std::string op = get_text(nodes[1]);
        ret = new Cond(new Cond(left),
                          new Cond(right), op);
      }
    }
    return ret;
  }
};

class Assertion {
public:
  Assertion(XMLNode assertnode) {
    // Precondition:
    // this is a call node
    // call_get_name returns "assert"
    // this is assert node, need to instrument the assert value as is.
    XMLNode argument = find_first_node_bfs(assertnode, "argument");
    argument = argument.child("expr");
    // I only want the items to be exactly:
    // - name, operator, name
    // - name
    // - literal is same as name
    std::vector<XMLNode> nodes;
    m_content = get_text(argument);
    for (XMLNode node : argument.children()) {
      nodes.push_back(node);
    }
    if (nodes.size() == 1) {
      // name!=0
      m_cond = new Cond(new Cond(get_text(nodes[0])),
                        new Cond("0"), "!=");
      if (std::string("name") == nodes[0].name()) {
        m_code = getcode(get_text(nodes[0]));
      }
    } else if (nodes.size() == 3) {
      if (std::string("operator") == nodes[1].name()) {
        std::string left = get_text(nodes[0]);
        if (left == "NULL") left = "0";
        std::string right = get_text(nodes[2]);
        if (right == "NULL") right = "0";
        std::string op = get_text(nodes[1]);
        m_cond = new Cond(new Cond(left),
                          new Cond(right), op);

        if (std::string("name") == nodes[0].name() && left != "0") {
          m_code += getcode(left);
        }
        if (std::string("name") == nodes[2].name() && left != "0") {
          m_code += getcode(right);
        }
      }
    }
  }
  ~Assertion() {
    delete m_cond;
  }

  std::string GetCode() {return m_code;}
  Cond *GetCond() {return m_cond;}
  std::string GetContent() {return m_content;}
private:
  std::string getcode(std::string name) {
    std::string fmt = "printf(\"%s=\\d\\n\", (int)%s);\n";
    char buf[BUFSIZ];
    sprintf(buf, fmt.c_str(), name.c_str(), name.c_str());
    std::string ret = buf;
    ret += "fflush(stdout);\n";
    return ret;
  }
  Cond *m_cond = NULL;
  std::string m_code;
  std::string m_content; // the content of the assertion, without assert()
};

#endif /* COND_H */
