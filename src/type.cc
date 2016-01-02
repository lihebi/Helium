#include "type.h"

/*
 * Get decl code for a pointer type
 */
std::string
Type::GetDeclCode(const std::string& type_name, const std::string& var_name, int pointer_level) {
  return type_name + std::string(pointer_level, '*')+ " " + var_name+";\n";
}

static std::string
qualify_var_name(const std::string& varname) {
  std::string tmp = varname;
  tmp.erase(std::remove(tmp.begin(), tmp.end(), '.'), tmp.end());
  tmp.erase(std::remove(tmp.begin(), tmp.end(), '>'), tmp.end());
  tmp.erase(std::remove(tmp.begin(), tmp.end(), '-'), tmp.end());
  return tmp;
}
/**
 * Only get the allocate code(malloc, assign), but no decl code.
 */
std::string
Type::GetAllocateCode(const std::string& type_name, const std::string& var_name, int pointer_level) {
  std::string code;
  std::string var_tmp = qualify_var_name(var_name) + "_tmp";
  code += type_name + "* " + var_tmp + " = (" + type_name + "*)malloc(sizeof(" + type_name + "));\n";
  code += var_name + " = " + std::string(pointer_level-1, '&') + var_tmp + ";\n";
  return code;
}

std::string
Type::GetArrayCode(const std::string& type_name, const std::string& var_name, int dimension) {
  // TODO only support 1 dimension!!!
  std::string code;
  std::string size_var = var_name + "_size";
  // std::string
  code += "int " + size_var + ";\n";
  code += "scanf(\"%d\", &"+size_var + ");\n";
  code += type_name + " " + var_name + "[" + size_var + "];\n";
  // TODO init code
  // code += "for (int i=0;i<" + size_var + ";i++) {\n";
  // code += "   scanf()"
  return code;
}

void
Type::SetDimension(int d) {
  m_dimension = d;
}
int
Type::GetDimension() const {
  return m_dimension;
}
void
Type::SetPointerLevel(int l) {
  m_pointer_level = l;
}
int
Type::GetPointerLevel() const {
  return m_pointer_level;
}

/*******************************
 ** Type factory
 *******************************/

static bool
search_and_remove(std::string &s, boost::regex reg) {
  if (boost::regex_search(s, reg)) {
    s = boost::regex_replace<boost::regex_traits<char>, char>(s, reg, "");
    return true;
  }
  return false;
}

static int
count_and_remove(std::string &s, char c) {
  int count = std::count(s.begin(), s.end(), c);
  if (count) {
    s.erase(std::remove(s.begin(), s.end(), c), s.end());
  }
  return count;
}

static void
fill_storage_specifier(std::string& name, struct storage_specifier& specifier) {
  specifier.is_auto     = search_and_remove(name, boost::regex("\\bauto\\b"))     ? 1 : 0;
  specifier.is_register = search_and_remove(name, boost::regex("\\bregister\\b")) ? 1 : 0;
  specifier.is_static   = search_and_remove(name, boost::regex("\\bstatic\\b"))   ? 1 : 0;
  specifier.is_extern   = search_and_remove(name, boost::regex("\\bextern\\b"))   ? 1 : 0;
  // specifier.is_typedef     = search_and_remove(name, typedef_regex)     ? 1 : 0;
}

static void
fill_type_specifier(std::string& name, struct type_specifier& specifier) {
  specifier.is_void     = search_and_remove(name, boost::regex("\\bvoid\\b"))     ? 1 : 0;
  specifier.is_char     = search_and_remove(name, boost::regex("\\bchar\\b"))     ? 1 : 0;
  specifier.is_short    = search_and_remove(name, boost::regex("\\bshort\\b"))    ? 1 : 0;
  specifier.is_int      = search_and_remove(name, boost::regex("\\bint\\b"))      ? 1 : 0;
  specifier.is_long     = search_and_remove(name, boost::regex("\\blong\\b"))     ? 1 : 0;
  specifier.is_float    = search_and_remove(name, boost::regex("\\bfloat\\b"))    ? 1 : 0;
  specifier.is_double   = search_and_remove(name, boost::regex("\\bdouble\\b"))   ? 1 : 0;
  specifier.is_signed   = search_and_remove(name, boost::regex("\\bsigned\\b"))   ? 1 : 0;
  specifier.is_unsigned = search_and_remove(name, boost::regex("\\bunsigned\\b")) ? 1 : 0;
  specifier.is_bool     = search_and_remove(name, boost::regex("\\bbool\\b"))     ? 1 : 0;
}
static void
fill_type_qualifier(std::string& name, struct type_qualifier& qualifier) {
  qualifier.is_const    = search_and_remove(name, boost::regex("\\bconst\\b"))    ? 1 : 0;
  qualifier.is_volatile = search_and_remove(name, boost::regex("\\bvolatile\\b")) ? 1 : 0;
}

static void
fill_struct_specifier(std::string& name, struct struct_specifier& specifier) {
  specifier.is_struct = search_and_remove(name, boost::regex("\\bstruct\\b")) ? 1 : 0;
  specifier.is_enum   = search_and_remove(name, boost::regex("\\benum\\b"))   ? 1 : 0;
  specifier.is_union  = search_and_remove(name, boost::regex("\\bunion\\b"))  ? 1 : 0;
}

/*
 * Type factory will handle:
 * - []
 * - *,&
 * - qualifier, specifier
 */
TypeFactory::TypeFactory(const std::string& name)
: m_name(name), m_dimension(0), m_pointer_level(0) {
  std::string name_tmp = m_name;
  if (name_tmp.find('[') != std::string::npos) {
    m_dimension = std::count(name_tmp.begin(), name_tmp.end(), '[');
    name_tmp = name_tmp.substr(0, name_tmp.find('['));
  }
  m_pointer_level = count_and_remove(name_tmp, '*');
  fill_storage_specifier(name_tmp, m_component.storage_specifier);
  fill_type_specifier(name_tmp, m_component.type_specifier);
  fill_type_qualifier(name_tmp, m_component.type_qualifier);
  fill_struct_specifier(name_tmp, m_component.struct_specifier);
  m_identifier = StringUtil::trim(name_tmp);
}

bool
TypeFactory::IsPrimitiveType() {
  return m_identifier.empty();
}

static bool
is_system_type(const std::string& identifier) {
  if (SystemResolver::Instance()->Has(identifier)) return true;
  else return false;
}

static bool
is_local_type(const std::string& identifier) {
  if (Ctags::Instance()->Parse(identifier).empty()) return false;
  else return true;
}

std::shared_ptr<Type>
TypeFactory::createLocalType() {
  std::shared_ptr<Type> type;
  // need to know the code for local type
  // only handle structure or typedef
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(m_identifier);
  // scan for the first pass, for 's' or 'g' snippets
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 's') {
      // just create the structure
      type = std::make_shared<StructureType>(m_identifier);
      break;
    } else if ((*it)->GetType() == 'g') {
      // this is the typedef enum
      type = std::make_shared<EnumType>(m_identifier);
    } else if ((*it)->GetType() == 'u') {
      type = std::make_shared<UnionType>(m_identifier);
    }
  }
  if (!type) {
    // separate 't' with 'gs' to fix the bug: typedef struct conn conn
    // resolve conn as the true code struct conn {} first possible, so that no recursive
    for (auto it=snippets.begin();it!=snippets.end();it++) {
      Snippet* s = *it;
      if (s->GetType() == 't') {
        if (((TypedefSnippet*)s)->GetTypedefType() == TYPEDEF_TYPE) {
          std::string to_type = ((TypedefSnippet*)s)->GetToType();
          std::string new_name = m_name;
          new_name.replace(new_name.find(m_identifier), m_identifier.length(), to_type);
          // CAUSION this may or may not be a primitive type
          // FIXME may infinite loop?
          // YES, by typedef struct conn conn
          if (new_name.empty()) break;
          type = TypeFactory(new_name).CreateType();
        } else if (((TypedefSnippet*)*it)->GetTypedefType() == TYPEDEF_FUNC_POINTER) {
          // function pointer??
        }
        break;
      }
    }
  }
  // type should contains something.
  // or it may be NULL. So if it fails, do not necessarily means a bug
  if (!type) {
    return NULL;
  }
  return type;
}

std::shared_ptr<Type>
TypeFactory::createSystemType() {
  std::shared_ptr<Type> type;
  std::string prim_type = SystemResolver::Instance()->ResolveType(m_identifier);
  if (prim_type.empty()) {
    // TODO detail of system type
    type = std::make_shared<SystemType>(m_identifier, m_component.struct_specifier);
  } else {
    std::string new_name = m_name;
    new_name.replace(new_name.find(m_identifier), m_identifier.length(), prim_type);
    // FIXME this should be a primitive type
    type = TypeFactory(new_name).CreateType();
  }
  return type;
}

std::shared_ptr<Type>
TypeFactory::CreateType() {
  std::shared_ptr<Type> type;
  if (IsPrimitiveType()) {
    type = std::make_shared<PrimitiveType>(m_component.type_specifier);
  } else if (is_local_type(m_identifier)) {
    type = createLocalType();
  } else if (is_system_type(m_identifier)) {
    type = createSystemType();
  } else {
    return NULL;
  }
  if (type) {
    type->SetPointerLevel(m_pointer_level);
    type->SetDimension(m_dimension);
  }
  return type;
}


/******************************
 ** EnumType
 ******************************/

EnumType::EnumType(const std::string& name) : m_name(name) {
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 'g') {
      m_snippet = *it;
      if ((*it)->GetName() == name) {
        m_name = name;
        m_avail_name = "enum " + m_name;
      } else {
        m_alias = name;
        m_avail_name = m_alias;
      }
    }
  }
  assert(!m_avail_name.empty());
}
EnumType::~EnumType() {
}
std::string
EnumType::GetInputCode(const std::string& var) const {
  // TODO parse fields
  // TODO input from outside
  return m_avail_name + " " + var + ";\n";
}
std::string
EnumType::GetInputCodeWithoutDecl(const std::string& var) const {
  return "";
}
std::string EnumType::GetOutputCode(const std::string& var) const {
  return "";
}
std::string
EnumType::GetInputSpecification() {
  return "";
}
std::string
EnumType::GetOutputSpecification() {
  return "";
}

/*******************************
 ** PrimitiveType
 *******************************/

PrimitiveType::PrimitiveType(const struct type_specifier& specifier)
: m_specifier(specifier) {
  // m_name only as a printable name
  if (specifier.is_signed)   m_name += "signed ";
  if (specifier.is_unsigned) m_name += "unsigned ";
  if (specifier.is_short)    m_name += "short ";
  if (specifier.is_long)     m_name += "long ";
  if (specifier.is_int)      m_name += "int ";
  if (specifier.is_char)     m_name += "char ";
  if (specifier.is_float)    m_name += "float ";
  if (specifier.is_double)   m_name += "double ";
  if (specifier.is_bool)     m_name += "bool ";
  if (specifier.is_void)     m_name += "void ";
  // should at least have some specifier
  assert(!m_name.empty());
  m_name.pop_back();
}

PrimitiveType::~PrimitiveType() {}

static std::string
get_input_decl(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  if (dimension>0) {
    return Type::GetArrayCode(type.c_str(), var, dimension);
  }
  if (pointer_level > 0) {
    code += Type::GetDeclCode(type, var, pointer_level);
    code += Type::GetAllocateCode(type, var, pointer_level);
  } else {
    code += type + " " + var + ";\n";
  }
  return code;
}

static std::string
get_input_scanf(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string assign;
  if (pointer_level > 0) {
    assign = std::string(pointer_level-1, '*');
  } else {
    assign = "&";
  }
  return "scanf(\"%" + formatter + "\", " + assign + var + ");\n";
}

static std::string
get_input(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  code += get_input_decl(type, formatter, var, pointer_level, dimension);
  code += get_input_scanf(type, formatter, var, pointer_level, dimension);
  return code;
}

static std::string
get_output(
  const std::string& type, const std::string& formatter,
  const std::string& var,
  int pointer_level, int dimension
) {
  std::string code;
  if (dimension > 0) {
    // code += "for (int _i=0;_i<"+var+"_size;_i++) {\n"
    //         "\tprintf(\"%" + formatter + "\\n\", " + var + "[_i]);\n}\n";
    // return code;
    // if it is array, we don't output anything
    return "";
  }
  if (pointer_level >0) {
    // if it is a pointer, only print if it is null
    // TODO print the value
    // This will pose difficulty on output specification.
    // This will also pose difficulty on run rate, because may cause buffer overflow
    return "printf(\"%d\\n\", ("+var+"==NULL));\n";
  }
  // code += "printf(\"%" + formatter + "\\n\", " + std::string(pointer_level, '*') + var + ");\n";
  code += "printf(\"%" + formatter + "\\n\", " + var + ");\n";
  return code;
}

static std::string
get_char_input(const std::string& var, int pointer_level, int dimension) {
  std::string code;
  // TODO array of pointers?
  if (dimension > 0) {
    return Type::GetArrayCode("char", var, dimension);
  }
  // pointer or not
  std::string assign;
  if (pointer_level > 0) {
    code += Type::GetDeclCode("char", var, pointer_level);
    code += Type::GetAllocateCode("char", var, pointer_level);
    assign = std::string(pointer_level-1, '*');
  } else {
    code += "char "+var+";\n";
    assign = "&";
  }
  code += "scanf(\"%c\", "+assign+var+");\n";
  return code;
}

std::string
get_void_input(const std::string& var, int pointer_level, int dimension) {
  if (pointer_level > 0) {
    return "void " + std::string(pointer_level, '*') + " " + var+" = NULL;\n";
  } else {
    return "";
  }
}

std::string
get_void_output(const std::string& var, int pointer_level, int dimension) {
  if (pointer_level>0) {
    return "printf(\"%d\\n\", ("+var+"==NULL));\n";
  } else {
    // this should never happen, because already exit in get_void_input
    return "";
  }
}

std::string
PrimitiveType::GetInputCode(const std::string& var) const {
  if (m_specifier.is_char) return get_char_input(var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_float) return get_input("float", "f", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_double) return get_input("double", "lf", var, GetPointerLevel(), GetDimension());
  // will not constrain bool here, but in input specification
  if (m_specifier.is_bool) return get_input("bool", "d", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_void) return get_void_input(var, GetPointerLevel(), GetDimension());
  // rest is int
  return get_input("int", "d", var, GetPointerLevel(), GetDimension());
}

std::string
PrimitiveType::GetInputCodeWithoutDecl(const std::string& var) const {
  if (m_specifier.is_char) return get_input_scanf("char", "c", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_float) return get_input_scanf("float", "f", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_double) return get_input_scanf("double", "lf", var, GetPointerLevel(), GetDimension());
  // will not constrain bool here, but in input specification
  if (m_specifier.is_bool) return get_input_scanf("bool", "d", var, GetPointerLevel(), GetDimension());
  // if (m_specifier.is_void) return get_void_input(var, GetPointerLevel(), GetDimension());
  // rest is int
  return get_input_scanf("int", "d", var, GetPointerLevel(), GetDimension());
}

std::string
PrimitiveType::GetOutputCode(const std::string& var) const {
  // TODO char output
  // if (m_specifier.is_char) return get_char_output(var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_float) return get_output("float", "f", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_double) return get_output("double", "lf", var, GetPointerLevel(), GetDimension());
  // will not constrain bool here, but in output specification
  if (m_specifier.is_bool) return get_output("bool", "d", var, GetPointerLevel(), GetDimension());
  if (m_specifier.is_void) return get_void_output(var, GetPointerLevel(), GetDimension());
  // rest is int
  return get_output("int", "d", var, GetPointerLevel(), GetDimension());
}

std::string
PrimitiveType::GetInputSpecification() {
  std::string spec;

  if (m_specifier.is_char) spec += "50_70,";
  else if (m_specifier.is_float) spec += "0_100,";
  else if (m_specifier.is_double) spec += "0_100,";
  else if (m_specifier.is_bool) spec += "0_1,";
  else if (m_specifier.is_void) ;
  else {
    spec += "0_255,";
  }
  if (GetDimension()>0) {
    // spec = "size," + spec + "endsize";
    // we only support one dimension, as well as no nested size
    spec = "size," + spec;
  }
  if (spec.back() == ',') spec.pop_back();
  return spec;
}

std::string
PrimitiveType::GetOutputSpecification() {
  if (GetDimension()>0) return "";
  else if (GetPointerLevel()>0) {return "NULL";}
  else {
    if (m_specifier.is_char) return "char";
    else if (m_specifier.is_float) return "float";
    else if (m_specifier.is_double) return "double";
    else if (m_specifier.is_bool) return "bool";
    else return "int";
  }
}

std::set<std::string> StructureType::m_recursion_set = std::set<std::string>();

StructureType::StructureType(const std::string& name) {
  // need to resolve instead of looking up registry,
  // because the resolving snippet phase
  // is far behind current phase of resolve IO variables.
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 's') {
      m_snippet = *it;
      m_name = m_snippet->GetName();
      break;
    }
  }
  assert(!m_name.empty());
  // enable parseFields will cause "fork: resource temporary unavailable", even on memcached.
  // not sure why, but it should be something about ThreadUtil
  // However threadutil is synchronized, so it should not have problem.
  // As a result, temporarily this parse field feature cannot be enabled.
  // But I'm pretty sure for large benchmarks this error should also occur.

  // if (m_recursion_set.find(m_name) != m_recursion_set.end()) {
  //   // if recursion detected, do not parse the field in current type, and init value set to null
  //   m_null = true;
  //   return;
  // } else {
  //   // before parseFields, push self to the stack
  //   m_recursion_set.insert(m_name);
  //   parseFields();
  //   // after parseFields, pop self from stack
  //   m_recursion_set.erase(m_name);
  // }
}
StructureType::~StructureType() {
}

std::string
StructureType::GetInputCode(const std::string& var) const {
  std::string code;
  if (GetDimension()>0) {
    code += Type::GetArrayCode(m_name, var, GetDimension());
  }
  if (GetPointerLevel()>0) {
    code += Type::GetDeclCode(m_name, var, GetPointerLevel());
    code += Type::GetAllocateCode(m_name, var, GetPointerLevel());
  } else {
    code += m_name + " " + var + ";\n";
  }
  // fields init
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {
    assert(GetPointerLevel()>=0);
    std::string prefix;
    if (GetPointerLevel()>0) {
      prefix = "("+std::string(GetPointerLevel(), '*')+var+").";
    } else {
      prefix = var+'.';
    }
    code += (*it)->GetInputCodeWithoutDecl(prefix);
  }
  return code;
}

std::string
StructureType::getPrefix(const std::string& var) const {
  std::string prefix;
  if (GetPointerLevel()>0) {
    prefix = "("+std::string(GetPointerLevel(), '*')+var+").";
  } else {
    prefix = var+'.';
  }
  return prefix;
}

std::string
StructureType::GetInputCodeWithoutDecl(const std::string& var) const {
  if (m_null) {
    return var + " = NULL;\n";
  }
  std::string code;
  if (GetPointerLevel()>0) {
    code += Type::GetAllocateCode(m_name, var, GetPointerLevel());
  }
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {

    code += (*it)->GetInputCodeWithoutDecl(getPrefix(var));
  }
  return code;
}

std::string
StructureType::GetOutputCode(const std::string& var) const {
  std::string code;
  if (GetDimension() > 0) {
    code += "// [StructureType::GetOutputCode] array code omitted.\n";
    return code;
  }
  if (GetPointerLevel()>0) {
    code += "printf(\"%d\", "+var+"==NULL);\n";
  }
  for (auto it=m_fields.begin();it!=m_fields.end();it++) {
    code += (*it)->GetOutputCode(getPrefix(var));
  }
  return code;
}
std::string
StructureType::GetInputSpecification() {
  return "";
}
std::string
StructureType::GetOutputSpecification() {
  return "";
}

void
StructureType::parseFields() {
  pugi::xml_document doc;
  SrcmlUtil::String2XML(m_snippet->GetCode(), doc);
  pugi::xml_node struct_node = doc.select_node("//struct").node();
  // pugi::xml_node name_node = struct_node.child("name");
  pugi::xml_node block_node = struct_node.child("block");
  for (pugi::xml_node decl_stmt_node : block_node.children("decl_stmt")) {
    std::shared_ptr<Variable> v = VariableFactory::FromDeclStmt(decl_stmt_node);
    if (v) {
      m_fields.push_back(v);
    }
  }
}

SystemType::SystemType(
  const std::string& name, const struct struct_specifier& specifier
) : m_name(name), m_specifier(specifier) {
  if (m_specifier.is_struct)     m_type = "struct " + m_name;
  else if (m_specifier.is_enum)  m_type = "enum " + m_name;
  else if (m_specifier.is_union) m_type = "union " + m_name;
  else                           m_type = m_name;
}
SystemType::~SystemType() {
}

std::string
SystemType::GetInputCode(const std::string& var) const {
  std::string code;
  if (GetDimension()>0) {
    code = Type::GetArrayCode(m_type.c_str(), var, GetDimension());
  }
  if (GetPointerLevel() > 0) {
    code = Type::GetAllocateCode(m_type, var, GetPointerLevel());
  } else {
    code = m_type + " " + var + ";\n";
  }
  return code;
}

std::string
SystemType::GetInputCodeWithoutDecl(const std::string& var) const {
  return "";
}

std::string
SystemType::GetOutputCode(const std::string& var) const {
  return "";
}
std::string
SystemType::GetInputSpecification() {
  return "";
}
std::string
SystemType::GetOutputSpecification() {
  return "";
}


UnionType::UnionType(const std::string& name) : m_name(name) {
  std::set<Snippet*> snippets = Ctags::Instance()->Resolve(name);
  for (auto it=snippets.begin();it!=snippets.end();it++) {
    if ((*it)->GetType() == 'u') {
      m_snippet = *it;
      if ((*it)->GetName() == name) {
        m_name = name;
        m_avail_name = "union " + m_name;
      } else {
        m_alias = name;
        m_avail_name = m_alias;
      }
    }
  }
  assert(!m_avail_name.empty());
}
UnionType::~UnionType() {
}
std::string
UnionType::GetInputCode(const std::string& var) const {
  // TODO parse fields
  // TODO input from outside
  std::string code;
  if (GetDimension()>0) {
    code = Type::GetArrayCode(m_avail_name.c_str(), var, GetDimension());
  }
  if (GetPointerLevel() > 0) {
    code = Type::GetAllocateCode(m_avail_name, var, GetPointerLevel());
  } else {
    code = m_avail_name + " " + var + ";\n";
  }
  return code;
}

std::string
UnionType::GetInputCodeWithoutDecl(const std::string& var) const {
  return "";
}

std::string UnionType::GetOutputCode(const std::string& var) const {
  return "";
}
std::string
UnionType::GetInputSpecification() {
  return "";
}
std::string
UnionType::GetOutputSpecification() {
  return "";
}

/*******************************
 ** Variable
 *******************************/


Variable::Variable(std::shared_ptr<Type> type, const std::string& name)
: m_type(type), m_name(name) {
}

std::string Variable::GetInputCode(const std::string& prefix) const {
  return m_type->GetInputCode(prefix+m_name);
}
std::string Variable::GetInputCodeWithoutDecl(const std::string& prefix) const {
  return m_type->GetInputCodeWithoutDecl(prefix+m_name);
}
std::string Variable::GetOutputCode(const std::string& prefix) const {
  return m_type->GetOutputCode(prefix+m_name);
}
std::string
Variable::GetInputSpecification() const {
  return m_type->GetInputSpecification();
}
std::string
Variable::GetOutputSpecification() const {
  return m_name + ":" + m_type->GetOutputSpecification();
}

std::vector<std::shared_ptr<Variable> >
VariableFactory::FromParamList(pugi::xml_node node) {
  std::vector<std::shared_ptr<Variable> > vvp;
  if (node.type() == pugi::node_element && strcmp(node.name(), "parameter_list") == 0) {
    for (pugi::xml_node param_node : node.children("param")) {
      std::shared_ptr<Variable> vp = FromDecl(param_node.child("decl"));
      if (vp) vvp.push_back(vp);
    }
  }
  return vvp;
}

std::vector<std::shared_ptr<Variable> >
VariableFactory::FromForInit(pugi::xml_node node) {
  std::vector<std::shared_ptr<Variable> > vvp;
  if (node.type() == pugi::node_element && strcmp(node.name(), "init") == 0) {
    // TODO support multiple variable in for init
    node = node.child("decl");
    std::shared_ptr<Variable> vp = FromDecl(node);
    if (vp) vvp.push_back(vp);
  }
  return vvp;
}

std::shared_ptr<Variable>
VariableFactory::FromDeclStmt(pugi::xml_node node) {
  if (node.type() == pugi::node_element && strcmp(node.name(), "decl_stmt") == 0) {
    node = node.child("decl");
    return FromDecl(node);
  }
  return NULL;
}

std::shared_ptr<Variable>
VariableFactory::FromDecl(pugi::xml_node node) {
  if (node.type() == pugi::node_element && strcmp(node.name(), "decl") == 0) {
    std::string type_str = DomUtil::GetTextContent(node.child("type"));
    std::string name_str = DomUtil::GetTextContent(node.child("name"));
    if (name_str.find('[') != std::string::npos) {
      type_str += name_str.substr(name_str.find('['));
    }
    name_str = name_str.substr(0, name_str.find('['));
    std::shared_ptr<Type> type = TypeFactory(type_str).CreateType();
    if (type) {
      std::shared_ptr<Variable> v = std::make_shared<Variable>(type, name_str);
      return v;
    } else {
      // std::cout<<"Type create not successful: " <<type_str<<std::endl;
    }
  }
  return NULL;
}
