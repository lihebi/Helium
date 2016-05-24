#include "builder.h"
#include <algorithm>
#include "utils.h"
#include "config.h"
#include "options.h"

#include <gtest/gtest.h>

using namespace ast;

Builder::Builder() {
  m_dir = utils::create_tmp_dir("/tmp/helium-test-tmp.XXXXXX");
}
Builder::~Builder() {}

// void
// Builder::writeMain() {
//   utils::write_file(Config::Instance()->GetString("output-folder")+"/generate.c", m_main);
// }

// void Builder::writeSupport() {
//   utils::write_file(
//     Config::Instance()->GetString("output-folder")+"/support.h",
//     m_support
//   );
// }

// void
// Builder::writeMakefile() {
//   utils::write_file(
//     Config::Instance()->GetString("output-folder")+"/Makefile",
//     m_makefile
//   );
// }

/**
 * from begin, resolve prev sibling and parent, for DeclStmt.
 * For every one, insert a guard variable.
 * Assert them right before begin and after end.
 * FIXME stack grow to lower address? add guard before variable decl?
 */
void add_stack_guard(Node begin, Node end) {
  int id;
  id = 1;
  Node node = begin;
  std::string stack_guard_assert = "\n// @HeliumStackGuardAssert\n";
  while (true) {
    if (helium_previous_sibling(node)) {
      node = helium_previous_sibling(node);
    } else if (helium_parent(node)) {
      node = helium_parent(node);
    } else {
      break;
    }
    if (kind(node) == NK_Function) break;
    if (kind(node) == NK_DeclStmt) {
      // constructing guard
      std::string var = "helium_stack_guard_"+std::to_string(id);
      std::string text = "\nint "+var+"="+std::to_string(id)+";\n";
      stack_guard_assert += "HELIUM_ASSERT("+var+"=="+std::to_string(id)+");\n";
      id++;
      // add guard
      // before the decl stmt, because the stack goes toward lower address
      Node new_node = node.parent().insert_child_before("helium_instrument", node);
      new_node.append_child(pugi::node_pcdata).set_value(text.c_str());
    }
  }
  // add guard assert
  Node new_node;
  new_node = begin.parent().insert_child_before("helium_instrument", begin);
  new_node.append_child(pugi::node_pcdata).set_value(stack_guard_assert.c_str());
  new_node = end.parent().insert_child_after("helium_instrument", end);
  new_node.append_child(pugi::node_pcdata).set_value(stack_guard_assert.c_str());
}

/**
 * In this Doc containing the nodes begin and end,
 * insert guard malloc after every malloc/calloc,
 * and assert them right before begin and after end.
 */
void add_heap_guard(Node begin, Node end) {
  int id=1;
  std::string heap_guard_decl = "\n// @HeliumHeapGuard\n";
  std::string heap_guard_assert = "\n// @HeliumHeapGuardAssert\n";
  NodeList calls = find_nodes_from_root(begin, NK_Call);
  for (Node call : calls) {
    if (call_get_name(call) == "malloc" || call_get_name(call) == "calloc") {
      Node stmt = helium_parent(call);
      // constructing guard code
      std::string var = "helium_heap_guard_"+std::to_string(id);
      heap_guard_decl += "int *"+var+";\n";
      heap_guard_decl += "bool " + var + "_valid=false;\n";
      // constructing definition
      std::string def;
      def += "\nif ("+var+"_valid) free("+var+");\n";
      def += var + "=(int*)malloc(sizeof(int));\n";
      def += "*"+var+"="+std::to_string(id)+";\n";
      def += var+"_valid=true;\n";
      // constructing assertion
      heap_guard_assert += "HELIUM_ASSERT(!"+var+"_valid || "+"*"+var+"=="+std::to_string(id)+");\n";
      id++;
      // add guard
      Node new_node = stmt.parent().insert_child_after("helium_instrument", stmt);
      new_node.append_child(pugi::node_pcdata).set_value(def.c_str());
    }
  }
  // all guard should be global pointer
  Node function = find_nodes_from_root(begin, NK_Function)[0];
  Node new_node = function.parent().insert_child_before("helium_instrument", function);
  new_node.append_child(pugi::node_pcdata).set_value(heap_guard_decl.c_str());
  // add guard assert
  new_node = begin.parent().insert_child_before("helium_instrument", begin);
  new_node.append_child(pugi::node_pcdata).set_value(heap_guard_assert.c_str());
  new_node = end.parent().insert_child_after("helium_instrument", end);
  new_node.append_child(pugi::node_pcdata).set_value(heap_guard_assert.c_str());
}

/**
 * Add print statement before and after segment.
 */
void add_print_guard(Node begin, Node end) {
  Node new_node;
  new_node = begin.parent().insert_child_before("helium_instrument", begin);
  new_node.append_child(pugi::node_pcdata).set_value("\nprintf(\"@HeliumBeforeSegment\\n\");\n");
  new_node = end.parent().insert_child_after("helium_instrument", end);
  new_node.append_child(pugi::node_pcdata).set_value("\nprintf(\"\\n@HeliumAfterSegment\\n\");\n");
}

/**
 * Add stack and heap guard.
 * The code text must contains @HeliumSegmentBegin and @HeliumSegmentEnd
 * Will add guard at both begin and end.
 */
std::string add_helium_guard(std::string text) {
  using namespace ast;
  ast::Doc doc;
  utils::string2xml(text, doc);
  Node segment_begin  = find_node_containing_str(doc, NK_Comment, "@HeliumSegmentBegin");
  Node segment_end = find_node_containing_str(doc, NK_Comment, "@HeliumSegmentEnd");
  // no change is no such annotation comments.
  if (!segment_begin || !segment_end) return text;
  // heap guard
  add_heap_guard(segment_begin, segment_end);
  // stack guard
  add_stack_guard(segment_begin, segment_end);
  NodeList callsites = find_nodes_containing_str(doc, NK_Comment, "@HeliumCallSite");
  for (Node callsite : callsites) {
    add_stack_guard(callsite, helium_next_sibling(callsite));
  }
  /**
   * TODO what about global and static variable?
   */

  add_print_guard(segment_begin, segment_end);

  return get_text(doc);
}

TEST(builder_test_case, DISABLED_helium_guard) {
  const char *raw = R"prefix(
int func() {
int a = malloc();
int b;
// @HeliumSegmentBegin
strcpy(a,b);
// @HeliumSegmentEnd
}
)prefix";
  const char *expect = R"prefix(

// @HeliumHeapGuard
int *helium_heap_guard_1;
int func() {
int a = malloc();
int helium_stack_guard_2=2;
helium_heap_guard_1=(int*)malloc(sizeof(int));
*helium_heap_guard_1=1;
int b;
int helium_stack_guard_1=1;
// @HeliumHeapGuardAssert
assert(*helium_heap_guard_1==1);
// @HeliumStackGuardAssert
assert(helium_stack_guard_1==1);
// @HeliumSegmentBegin
strcpy(a,b);
// @HeliumSegmentEnd
// @HeliumHeapGuardAssert
assert(*helium_heap_guard_1==1);
// @HeliumStackGuardAssert
assert(helium_stack_guard_1==1);
}
)prefix";
  std::string result = add_helium_guard(raw);
  EXPECT_EQ(result, std::string(expect));
  std::cout <<result  << "\n";
}

void
Builder::Write() {
  /*******************************
   ** Outputing code
   *******************************/
  // std::string main_text = m_seg->GetMain();
  // std::string support = m_seg->GetSupport();
  // std::string makefile = m_seg->GetMakefile();

  // std::cout <<utils::BLUE <<m_main  << utils::RESET << "\n";
  if (Config::Instance()->GetString("helium-guard") == "true") {
    m_main = add_helium_guard(m_main);
  }

  utils::write_file(m_dir+"/main.c", m_main);
  utils::write_file(m_dir+"/support.h", m_support);
  utils::write_file(m_dir + "/Makefile", m_makefile);

  for (std::pair<std::string, std::string> pair : m_scripts) {
    utils::write_file(m_dir+"/"+pair.first, pair.second);
  }

  // std::cout <<utils::BLUE <<main_text  << utils::RESET << "\n";
}

std::string simplify_error_msg(std::string error_msg) {
  std::vector<std::string> msgs = utils::split(error_msg, '\n');
  std::string result;
  for (std::string msg : msgs) {
    if (msg.find("error") != std::string::npos) {
      result += msg+"\n";
    }
  }
  return result;
}
void
Builder::Compile() {
  std::string clean_cmd = "make clean -C " + m_dir;
  std::string cmd = "make -C " + m_dir;
  cmd += " 2>&1";
  utils::exec(clean_cmd.c_str(), NULL);
  int return_code;
  std::string error_msg = utils::exec(cmd.c_str(), &return_code);
  if (return_code == 0) {
    m_success = true;
  } else {
    m_success = false;
    if (PrintOption::Instance()->Has(POK_CompileError)) {
      utils::print(simplify_error_msg(error_msg), utils::CK_Yellow);
    }
  }
}

std::string Builder::GetExecutable() {
  // return Config::Instance()->GetString("output-folder") + "/a.out";
  return m_dir + "/a.out";
}
