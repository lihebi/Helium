#include "builder.h"
#include <algorithm>
#include "utils/utils.h"
#include "helium_options.h"

#include "resolver/system_resolver.h"

#include "utils/log.h"
#include "parser/xml_doc_reader.h"
#include "parser/xmlnode_helper.h"
#include <gtest/gtest.h>

Builder::Builder() {
  m_dir = utils::create_tmp_dir("/tmp/helium-test-tmp.XXXXXX");
}
Builder::~Builder() {}

/**
 * from begin, resolve prev sibling and parent, for DeclStmt.
 * For every one, insert a guard variable.
 * Assert them right before begin and after end.
 * FIXME stack grow to lower address? add guard before variable decl?
 */
void add_stack_guard(XMLNode begin, XMLNode end) {
  int id;
  id = 1;
  XMLNode node = begin;
  std::string stack_guard_assert = "\n// @HeliumStackGuardAssert\n";
  while (true) {
    if (helium_previous_sibling(node)) {
      node = helium_previous_sibling(node);
    } else if (helium_parent(node)) {
      node = helium_parent(node);
    } else {
      break;
    }
    if (xmlnode_to_kind(node) == NK_Function) break;
    if (xmlnode_to_kind(node) == NK_DeclStmt) {
      // constructing guard
      std::string var = "helium_stack_guard_"+std::to_string(id);
      std::string text = "\nint "+var+"="+std::to_string(id)+";\n";
      stack_guard_assert += "HELIUM_ASSERT("+var+"=="+std::to_string(id)+");\n";
      id++;
      // add guard
      // before the decl stmt, because the stack goes toward lower address
      XMLNode new_node = node.parent().insert_child_before("helium_instrument", node);
      new_node.append_child(pugi::node_pcdata).set_value(text.c_str());
    }
  }
  // add guard assert
  XMLNode new_node;
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
void add_heap_guard(XMLNode begin, XMLNode end) {
  int id=1;
  std::string heap_guard_decl = "\n// @HeliumHeapGuard\n";
  std::string heap_guard_assert = "\n// @HeliumHeapGuardAssert\n";
  XMLNodeList calls = find_nodes_from_root(begin, NK_Call);
  for (XMLNode call : calls) {
    if (call_get_name(call) == "malloc" || call_get_name(call) == "calloc") {
      XMLNode stmt = helium_parent(call);
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
      XMLNode new_node = stmt.parent().insert_child_after("helium_instrument", stmt);
      new_node.append_child(pugi::node_pcdata).set_value(def.c_str());
    }
  }
  // all guard should be global pointer
  XMLNode function = find_nodes_from_root(begin, NK_Function)[0];
  XMLNode new_node = function.parent().insert_child_before("helium_instrument", function);
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
void add_print_guard(XMLNode begin, XMLNode end) {
  XMLNode new_node;
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
  XMLDoc doc;
  utils::string2xml(text, doc);
  XMLNode segment_begin  = find_node_containing_str(doc, NK_Comment, "@HeliumSegmentBegin");
  XMLNode segment_end = find_node_containing_str(doc, NK_Comment, "@HeliumSegmentEnd");
  // no change is no such annotation comments.
  if (!segment_begin || !segment_end) return text;
  // heap guard
  add_heap_guard(segment_begin, segment_end);
  // stack guard
  add_stack_guard(segment_begin, segment_end);
  XMLNodeList callsites = find_nodes_containing_str(doc, NK_Comment, "@HeliumCallSite");
  for (XMLNode callsite : callsites) {
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

/**
 * FIXME
 */
static std::string remove_line_marker(std::string s) {
  std::string ret;
  std::vector<std::string> sp = utils::split(s, '\n');
  for (std::string line : sp) {
    if (!line.empty() && line[0] == '#') {
      std::string tmp = line.substr(1);
      utils::trim(tmp);
      std::vector<std::string> v = utils::split(tmp);
      if (v.size() == 0) continue;
      if (utils::is_number(v[0])) {
        continue;
      }
      // if (line.find("include") == std::string::npos
      //     && line.find("define") == std::string::npos) {
      //   continue;
      // }
    }
    ret += line + "\n";
  }
  return ret;
}

void Builder::preProcess() {
  // main and support, remove line markers
  m_main = remove_line_marker(m_main);
  m_support = remove_line_marker(m_support);
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
  // if (HeliumOptions::Instance()->GetBool("instrument-helium-guard")) {
  //   m_main = add_helium_guard(m_main);
  // }

  preProcess();

  utils::write_file(m_dir+"/main.c", m_main);
  utils::write_file(m_dir+"/main.h", m_support);
  utils::write_file(m_dir + "/Makefile", m_makefile);

  // copy the test script
  std::string script_file = utils::escape_tide(HeliumOptions::Instance()->GetString("helium-home"))
    + "/scripts/helium-test-segment.sh";
  // script_file = "/home/hebi/github/helium/scripts/helium-test-segment.sh";
  // std::cout << "copying the script: " << script_file << "\n";
  if (!fs::exists(script_file)) {
    std::cerr << "EE: fatal error: test script does not exist."
              << "Did you set the correct helium-home option in helium config file?" << "\n";
    exit(1);
  }
  fs::copy(script_file, m_dir+"/test.sh");
  // utils::write_file(m_dir+"/test.sh");

  // post process the outputed files: main.c, main.h, Makefile
  postProcess();

  for (std::pair<std::string, std::string> pair : m_scripts) {
    utils::write_file(m_dir+"/"+pair.first, pair.second);
  }

  // std::cout <<utils::BLUE <<main_text  << utils::RESET << "\n";
}

// void instrument_free(std::string file) {
//   XMLDoc *doc = XMLDocReader::CreateDocFromFile(file);
//   XMLNode root = doc->document_element();
//   for (XMLNode call_node : root.select_nodes("//call")) {
//     std::string func = call_node.child_value("name");
//     if (func == "free") {
      
//     }
//   }
//   delete doc;
// }

void global_add_static(XMLDoc *doc) {
  XMLNode root = doc->document_element();
  for (pugi::xpath_node p_node : root.select_nodes("/unit/decl_stmt")) {
    XMLNode decl_stmt_node = p_node.node();
    // <decl_stmt><decl><specifier>static</specifier>
    std::string specifier = decl_stmt_node.child("decl").child("specifier").child_value();
    if (specifier != "static") {
      // add static!
      XMLNode new_specifier = decl_stmt_node.prepend_child("helium_specifier");
      new_specifier.append_child(pugi::node_pcdata).set_value("static ");
    }
  }
}

// TEST(BuilderTestCase, global_add_static_test) {
//   XMLDoc *doc = XMLDocReader::CreateDocFromFile("/tmp/helium-test-tmp.SX2ZoL/main.h");
// }

void remove_typedef(XMLNode node) {
  for (pugi::xpath_node typedef_n : node.select_nodes("//typedef")) {
    XMLNode typedef_node = typedef_n.node();
    std::string name = typedef_node.child("name").child_value();
    std::string sys = SystemResolver::Instance()->ResolveType(name);
    if (!sys.empty()) {
      typedef_node.parent().remove_child(typedef_node);
    }
  }
}

/**
 * I would like do the free-d list instumentation here
 */
void Builder::postProcess() {
  // process m_dir+"/main.c"
  // instrument_free(m_dir+"/main.c");
  // process m_dir+"/main.h"
  // instrument_free(m_dir+"/main.h");
  // process m_dir+"/Makefile"

  // For main.c and main.h, get all global variables, and turns them to be static if they are not.
  XMLDoc *doc = XMLDocReader::CreateDocFromFile(m_dir + "/main.c");
  global_add_static(doc);
  std::string code = get_text(doc->document_element());
  utils::write_file(m_dir + "/main.c", code);
  delete doc;
  // main.h
  doc = XMLDocReader::CreateDocFromFile(m_dir + "/main.h");
  global_add_static(doc);
  code = get_text(doc->document_element());
  utils::write_file(m_dir + "/main.h", code);
  delete doc;

  // remove typedef if system already de
  // FIXME this might remove some that is needed by the header is not included
  // FIXME For IO type, the insturmented declaration will be the resolved name
  // TODO output many versions, every one success counts
  // doc = XMLDocReader::CreateDocFromFile(m_dir + "/main.h");
  // remove_typedef(doc->document_element());
  // code = get_text(doc->document_element());
  // utils::write_file(m_dir + "/main.h", code);
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
  std::string error_msg = utils::exec_sh(cmd.c_str(), &return_code);
  if (return_code == 0) {
    m_success = true;
  } else {
    m_success = false;
    helium_dump_compile_error("error: " + m_dir);
    helium_dump_compile_error(simplify_error_msg(error_msg));
    if (HeliumOptions::Instance()->GetBool("print-compile-error")) {
      std::cout << utils::YELLOW << simplify_error_msg(error_msg) << utils::RESET << "\n";
    }
  }
}

std::string Builder::GetExecutable() {
  // return Config::Instance()->GetString("output-folder") + "/a.out";
  return m_dir + "/a.out";
}
