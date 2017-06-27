#include "helium/type/type.h"
#include "helium/type/composite_type.h"
#include "helium/resolver/snippet_db.h"
#include "helium/parser/resource.h"
#include "helium/type/io_helper.h"

StructType::StructType(std::string raw, int snippet_id)
  : m_raw(raw), m_snippet_id(snippet_id) {
  std::cout << "StructType: " << raw << "\n";
  // std::string code = SnippetDB::Instance()->GetCode(snippet_id);
}
std::string StructType::GetDeclCode(std::string var) {
  // Decl code
  std::string ret;
  ret += "// StructType::GetDeclCode: " + var + "\n";
  ret += m_raw + " " + var + ";\n";
  return ret;
}

void StructType::GenerateIOFunc() {
  std::string key = IOHelper::ConvertTypeStr(m_raw);
  if (IOHelper::Instance()->Has(key)) return;
  std::string input,output;
  /**
   * Input
   */
  input += "void input_" + key + "(" + m_raw + "*var) {\n";
  output += "void output_" + key + "(" + m_raw + "*var, const char *name) {\n";
  StructClass *sc = StructResource::Instance()->GetStruct(m_snippet_id);
  for (Decl *decl : sc->Fields()) {
    Type *t = decl->GetType();
    std::string name = decl->GetName();
    if (t) {
      input += "  " + t->GetInputCode("var->"+name);
      t->GenerateIOFunc();
      std::string key = IOHelper::ConvertTypeStr(t->GetRaw());

      // use sprintf to construct the name
      output += "  sprintf(hbuf, \"%s->" + name + "\", name);\n";
      output += "  output_" + key + "(&(var->" + name + "), hbuf);\n";
    }
  }
  input += "}\n";
  output += "}\n";
  IOHelper::Instance()->Add(key, input, output);
}

std::string StructType::GetInputCode(std::string var) {
  // the input code should initiate all the fields
  std::string key = IOHelper::ConvertTypeStr(m_raw);
  GenerateIOFunc();
  return IOHelper::GetInputCall(key, var);
}
std::string StructType::GetOutputCode(std::string var) {
  // output each field
  std::string key = IOHelper::ConvertTypeStr(m_raw);
  GenerateIOFunc();
  return IOHelper::Instance()->GetOutputCall(key, var, var);
}
InputSpec* StructType::GenerateRandomInput() {
  StructInputSpec *ret = new StructInputSpec();
  StructClass *sc = StructResource::Instance()->GetStruct(m_snippet_id);
  for (Decl *decl : sc->Fields()) {
    Type *t = decl->GetType();
    std::string name = decl->GetName();
    if (t) {
      InputSpec *spec = t->GenerateRandomInput();
      ret->AddField(name, spec);
    }
  }
  return ret;
}
std::string StructType::GetRaw() {
  return m_raw;
}


std::string StructInputSpec::GetRaw() {
  std::string ret;
  std::vector<std::string> list;
  for (auto m : m_fields) {
    InputSpec *spec = m.second;
    if (spec) {
      list.push_back(spec->GetRaw());
    }
  }
  std::string joined = boost::algorithm::join(list, " ");
  ret = joined;
  return ret;
}

std::string StructInputSpec::GetSpec() {
  std::string ret;
  ret += "{";
  std::vector<std::string> list;
  for (auto m : m_fields) {
    std::string name = m.first;
    InputSpec *spec = m.second;
    if (spec) {
      list.push_back(name + ": " + spec->GetSpec());
    }
  }
  std::string joined = boost::algorithm::join(list, ", ");
  ret += joined;
  ret += "}";
  return ret;
}

