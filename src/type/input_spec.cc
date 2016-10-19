#include "input_spec.h"

std::string PointerInputSpec::GetRaw() {
  std::string ret;
  int helium_size = m_heap.size();
  ret += std::to_string(helium_size) + " ";
  for (int i=0;i<helium_size;i++) {
    // ret += m_heap[i]->GetRaw() + " ";
    if (m_heap[i]) {
      ret += m_heap[i]->GetRaw() + " ";
    }
  }
  return ret;
}

std::string PointerInputSpec::GetSpec() {
  std::string ret;
  ret += "{helium_siz: ";
  ret += std::to_string(m_heap.size());
  ret += ", heap: {";
  for (InputSpec *spec : m_heap) {
    if (spec) {
      ret += spec->GetSpec();
      ret += ",";
    }
  }
  ret += "} }";
  return ret;
}
