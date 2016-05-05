#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <fstream>

#include "main.h"
#include "../../src/ast.h"
#include "../../src/utils.h"
#include "../../src/ast_node.h"
#include "../../src/common.h"
#include "../../src/type.h"


#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace po = boost::program_options;

using namespace ast;
SlicingCriteria::SlicingCriteria(std::string abs, int l)
  : m_abs_filename(abs), m_linum(l) {
  // std::cout << "SlicingCriteria::Constructor"  << "\n";
  /**
   * Parse file
   */
  // 1. parse the file, locate the function node
  m_doc = utils::file2xml(m_abs_filename);
  pugi::xml_node root = m_doc->document_element();
  // XMLNodeList func_nodes = find_nodes(root, NK_Function);
  m_func = find_node_enclosing_line(root, NK_Function, m_linum);
  assert(m_func &&
         "Take it easy, the slicing criteria may not be in a function. Just continue to the next record.");
  // int func_begin_line = get_node_line(m_func);
  // int func_end_line = get_node_last_line(m_func);
  // std::cout << "creating AST for func, begin: " << func_begin_line << " end: " << func_end_line  << "\n";
  // std::cout << "criteria linum: " << l  << "\n";
  m_ast = new AST(m_func);
  // 2. resovle LHS and RHS variables (only the type kind, not the actual type)
  m_astnode = m_ast->GetNodeByLinum(m_linum);
  assert(m_ast);
  assert(m_astnode);
  /**
   * Get the line content into m_line
   */
  std::ifstream is;
  is.open(m_abs_filename);
  if (is.is_open()) {
    std::string line;
    int count=0;
    while (std::getline(is, line)) {
      count ++;
      if (count == m_linum) {
        m_line = line;
        break;
      }
    }
  }
  is.close();
  resolveVars();
}

/**
 * Resolve variables used in this slicing criteria
 * 1. get the ids used in the line
 * 2. from astnode, resolve the ids
 * 3. For those resolved, record their name and type (in terms of MLSliceTypeKind)
 * TODO LHS and RHS
 */
void SlicingCriteria::resolveVars() {
  // std::cout << "resolveVars"  << "\n";
  std::set<std::string> ids = extract_id_to_resolve(m_line);
  SymbolTable *sym = m_astnode->GetSymbolTable();
  assert(sym);
  for (std::string id : ids) {
    SymbolTableValue *value = sym->LookUp(id);
    if (!value) continue;
    std::string name = value->GetName();
    std::string type = value->GetType();
    /**
     *
     typedef enum {
     MLST_Pointer,
     MLST_Reference,
     MLST_Array,
     MLST_Class,
     MLST_Struct,
     MLST_Primitive
     } MLSliceTypeKind;
     */
    // FIXME the type can be combination of the above properties, e.g. array of pointer to primitive
    MLSliceType t;
    if (is_primitive(type)) t.SetPrimitive();
    else t.SetStruct();
    if (type.find('&') != std::string::npos) t.SetReference();
    if (type.find('[') != std::string::npos) t.SetArray();
    if (type.find('*') != std::string::npos) t.SetPointer();
    unsigned int t_i = t.GetValue();
    m_sym_tbl[name] = t_i;
  }
  // std::cout << "end of resolve vars"  << "\n";
}

int SlicingCriteria::GetFuncBeginLinum() {
  // std::cout << "GetFuncBeginlinum"  << "\n";
  return get_node_line(m_func);
}
int SlicingCriteria::GetFuncEndLinum() {
  // std::cout << "GetFuncEndLinum"  << "\n";
  return get_node_last_line(m_func);
}

std::vector<FeatureRecord>
compute_features(SlicingCriteria *criteria, std::set<int> slice_linums,
                 std::vector<std::pair<std::string, int> > candidates) {
  // std::cerr << "compute features"  << "\n";
  std::vector<FeatureRecord> ret;
  /**
   * Computing the global variables
   */
  std::set<std::string> globals;
  pugi::xml_node xmlnode = criteria->GetXMLFuncNode();
  // if (xmlnode) {
  //   std::cerr << "yes" << "\n";
  // }
  pugi::xpath_node_set global_name_nodes = xmlnode.root().select_nodes("/unit/decl_stmt/decl/name");
  // std::cerr << global_name_nodes.size() << "\n";
  for (auto pn : global_name_nodes) {
    XMLNode node = pn.node();
    std::string name = node.child_value();
    if (!name.empty()) {
      globals.insert(name);
    }
  }
  // std::cerr << criteria->GetAbsFilename() << "\n";
  // std::cerr << globals.size()  << "\n";

  /**
   * Now line is the target(T), criteria is the POI
   * Compute the features for T, with regard to POI
   */
  // TODO features
  // f1 variable name
  // f2 variable type
  // f3: distance
  // f4: AST level
  // f5: Transformation Count
  // std::cerr << candidates.size() << "\n";
  int size = candidates.size();
  // if (size > 100) {
  //   std::cerr << "larger than 100, return" << "\n";
  //   return {};
  // }
  for (int idx=0;idx<size;idx++) {
    // std::cerr << idx << "\n";
    /**
     * Basic informations
     */
    FeatureRecord record;
    std::string line = candidates[idx].first;
    int linum = candidates[idx].second;
    record.linum = linum;
    record.filename = criteria->GetAbsFilename();
    record.distance = criteria->GetLinum() - linum;
    // std::cout << idx  << "\n";
    std::set<std::string> ids = extract_id_to_resolve(line);
    // check whether the variable is global variable
    // check whether it contains a function call
    for (std::string id : ids) {
      if (globals.count(id) == 1) {
        record.use_global = true;
      }
    }

    /***********
     * The followings are features that requrie the T and POI in the same function
     ***********/
    // map the record to an ASTNode*
    ASTNode *astnode = criteria->GetAST()->GetNodeByLinum(linum);
    /**
     *
     * FIXME the line may be only a brace, e.g.
     * if ()
     * { // this is the line
     * ...
     * in this case, we will only have the IF element for it,
     * but it should not be in the slice since it contains no more than a brace
     * FIXME Another issue is the statement may split into lines.
     * It is hard to identify the variables, and resolve them, if we don't map them to the AST nodes.
     * But if we do, we cannot know the information exact.
     * OK, but for now, I'm only using the identifier, be it a variable name, structure name, or a function name.
     * Or, I only need to resolve the type of that variable.
     * It should be efficient since I have symbol table instead of looking up the AST XML node every time.
     */
    // assert(astnode); // this is very likely to fail
    if (!astnode) continue;
    // AST distance
    record.AST_distance = criteria->GetAST()->Distance(criteria->GetASTNode(), astnode);
    ASTNode *lca = criteria->GetAST()->ComputeLCA(std::set<ASTNode*>{criteria->GetASTNode(), astnode});
    record.ast_dis_t_lca = criteria->GetAST()->Distance(astnode, lca);
    record.ast_dis_poi_lca = criteria->GetAST()->Distance(criteria->GetASTNode(), lca);
    record.ast_lca_lvl = lca->GetLevel();
    if (slice_linums.count(linum) == 1) record.in_slice=true;
    /**
     * The variable used.
     * FIXME what if multiple variables are used?
     */
    for (std::string id : ids) {
      if (criteria->LookUpVariable(id) != 0) {
        record.var_name_used_in_POI = true;
        record.var_type = criteria->LookUpVariable(id);
      }
    }
    ret.push_back(record);
  }
  return ret;
}


// /**
//  * Save the record
//  * Output format for mlslice:
//  * The features

//  * For each line:
//  * <ID>, <filename>, <line number> <f1>, <f2>, <f3>, true(false)
//  */
/**
 * @param [in] slice_linums the line numbers that are 1) in the slice 2) belong the the same file, same function
 */
std::vector<std::string> solve(SlicingCriteria *criteria, std::set<int> slice_linums) {
  // std::cerr << "solve"  << "\n";
  std::vector<std::string> ret;
  /**
   * get all lines in the same function, process from there
   */
  std::ifstream is;
  is.open(criteria->GetAbsFilename());
  assert(is.is_open());
  /**
   * Get the lines as candidates
   * Gathering them into such a vector because I need to build the transformation information efficiently
   */
  std::vector<std::pair<std::string, int> > candidates;
  std::string line;
  int linum=0;
  int c_begin = criteria->GetFuncBeginLinum();
  int c_end = criteria->GetFuncEndLinum();
  while (std::getline(is, line)) {
    linum++;
    // Now I comment these two lines out, because I want to analyze interprocedure ones
    if (linum < c_begin) continue;
    if (linum > c_end) break;
    candidates.push_back({line, linum});
  }
  is.close();

  std::vector<FeatureRecord> records = compute_features(criteria, slice_linums, candidates);
  for (FeatureRecord record : records) {
    ret.push_back(record.dump());
  }
  return ret;
}

/**
 * for the given slice_file and benchmark folder, do MLSlice, dump features
 */
void mlslice(std::string slice_file, std::string benchmark_folder) {
  // std::cerr << "mlslice"  << "\n";
  std::ifstream is;
  is.open(slice_file);
  SlicingCriteria *criteria=NULL;
  std::set<int> slice_linums;
  assert(is.is_open());
  std::string criteria_line;
  std::getline(is, criteria_line);
  /**
   * Getting information about slicing criteria
   */
  assert(criteria_line.find(':') != std::string::npos);
  int slicing_linum = atoi(criteria_line.substr(criteria_line.find(':')+1).c_str());
  std::string slicing_file = criteria_line.substr(0, criteria_line.find(':'));

  // gzip slow line
  if (slicing_file == "lib/vasnprintf.c") {
    std::cerr << "lib/vasnprintf.c ... skipped\n";
    return;
  }
    
  std::string true_file = benchmark_folder+"/"+ slicing_file;
  // std::cout << true_file  << "\n";
  // FIXME this will fail when I use ~/xxx
  assert(fs::exists(true_file));
  std::string abs_filename =  fs::canonical(true_file).string();
  // std::cout << "--- " << abs_filename   << "\n";
  criteria = new SlicingCriteria(abs_filename, slicing_linum);
  /**
   * Gather The slices
   */
  std::string line;
  while (std::getline(is, line)) {
    // std::cout << line  << "\n";
    std::vector<std::string> v = utils::split(line);
    assert(v.size() == 2);
    std::string file = v[0];
    if (file.empty()) continue;
    // only accept relative path!
    // this is actually used to eliminate /usr/include staff
    if (file[0] == '/') continue;
    std::string true_file = benchmark_folder + "/" + file;
    // std::cout << true_file  << "\n";
    assert(fs::exists(true_file));
    std::string abs_filename = fs::canonical(true_file).string();
    // std::cout << abs_filename  << "\n";
    int linum = atoi(v[1].c_str());
    // only retain if it is in the same function
    if (abs_filename != criteria->GetAbsFilename()) continue;
    if (linum < criteria->GetFuncBeginLinum() || linum > criteria->GetFuncEndLinum()) continue;
    slice_linums.insert(linum);
  }
  is.close();
  std::vector<std::string> records = solve(criteria, slice_linums);
  std::cout << FeatureRecord::header()  << "\n";
  for (std::string record : records) {
    std::cout << record  << "\n";
  }
}



/**
 *
Variable name
Variable type
distance
AST level
Transformation Count
Transformation Count Closure
 */


/**
 * 

1. Read from slice.txt, containing a single slice criteria and its slices.
2. open the file, locate the function containing the criteria
3. compute meta for the criteria => POI
  - RHS variable name used
  - LHS
  - AST level
  - line number
4. for each line in the function, compute:  => T
  - variable used
  - type of that variable
  - line number
  - AST level
  - LHS
5. from T to POI, compute the usage of the LHS of T

6 output

 */
int main(int argc, char *argv[]) {
  po::options_description options("Arguments");
  options.add_options()
    ("help,h", "produce help message") // --help, -h
    ("slice", po::value<std::string>(),
     "slice file containing single slice (with relative path to /benchmark/ folder)")
    ("benchmark", po::value<std::string>(), "benchmark folder")
    ;
  // positional options: used for the option that don't need to use a --prefix
  po::positional_options_description positional;
  // this "folder" option include only one item
  positional.add("folder", 1);
  
  /* run parser and store in m_vm */
  po::variables_map vm;
  po::store(
            po::command_line_parser(argc, argv)
            .options(options) // add cmdline options
            .positional(positional)     // add positional options
            .run(),                     // run the parser
            // store into m_vm
            vm
            );
  po::notify(vm);

  if (vm.count("help") == 1) {
    std::cout << options  << "\n";
    exit(1);
  }
  if (vm.count("slice") == 0 || vm.count("benchmark") == 0) {
    std::cout << options  << "\n";
    exit(1);
  }

  /**
   * mlslice --slice=/path/to/slice.txt --benchmark=/path/to/benchmark
   * mlslice /path/to/single/slice.txt --
   * In this case, I even don't need the program_options ..

   * FORMAT:
   * Ensure the slice.txt contains only one slice criteria.
   * All the filenames in slice.txt should be RELATIVE path

   * The format should be:

   filename:8
   slice1.c 12
   slice2.c 48
   slice5.c 34
   */

  std::string slice_file = vm["slice"].as<std::string>();
  std::string benchmark_folder = vm["benchmark"].as<std::string>();


  mlslice(slice_file, benchmark_folder);
}
