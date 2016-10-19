#ifndef INPUT_SPEC_H
#define INPUT_SPEC_H

#include "common.h"

/**
 * The format of spec.

 Wait, this is very hard to be consistent.
 I'd rather print out the input spec as well.
 
 */

/**
 * The spec for a chain of type is hard. But I have solution.
 * The spec format is a like-"json" format: use {} for the level.
 */
class InputSpec {
public:
  // FIXME This is very bad practice: the accept two strings, it is easy to confuse the position.
  InputSpec(std::string spec="", std::string raw="") : m_spec(spec), m_raw(raw) {}
  virtual ~InputSpec() {}
  virtual void Add(InputSpec *spec) {
    // Should not reach here.
    assert(false && "Should not reach here.");
    if (spec) {}
  }
  virtual std::string GetSpec() {
    return m_spec;
  }
  virtual std::string GetRaw() {
    return m_raw;
  }

private:
  std::string m_spec;
  std::string m_raw;
};

class ArrayInputSpec : public InputSpec {
public:
  ArrayInputSpec() {}
  virtual ~ArrayInputSpec() {
    for (InputSpec *spec : m_specs) {
      if (spec) delete spec;
    }
  }
  virtual std::string GetSpec() override {
    std::string ret;
    ret += "[";
    std::vector<std::string> list;
    for (InputSpec *spec : m_specs) {
      list.push_back(spec->GetSpec());
    }
    std::string joined = boost::algorithm::join(list, ", ");
    ret += joined;
    ret += "]";
    return ret;
  }

  virtual std::string GetRaw() override {
    std::string ret;
    std::vector<std::string> list;
    for (InputSpec *spec : m_specs) {
      list.push_back(spec->GetRaw());
    }
    std::string joined = boost::algorithm::join(list, " ");
    ret += joined;
    return ret;
  }
  
  virtual void Add(InputSpec *spec) override {
    m_specs.push_back(spec);
  }
private:
  std::vector<InputSpec*> m_specs;
};

class PointerInputSpec : public InputSpec {
public:
  PointerInputSpec() {}
  virtual ~PointerInputSpec() {
    for (InputSpec* spec : m_heap) {
      if (spec) delete spec;
    }
  }
  virtual std::string GetRaw() override;
  virtual std::string GetSpec() override;
  virtual void Add(InputSpec *spec) override {
    m_heap.push_back(spec);
  }
private:
  std::vector<InputSpec*> m_heap;
};

class StructInputSpec : public InputSpec {
public:
  StructInputSpec() {}
  virtual ~StructInputSpec() {}

  virtual std::string GetRaw() override;
  virtual std::string GetSpec() override;
  
  void AddField(std::string name, InputSpec *spec) {
    m_fields.push_back({name, spec});
  }
private:
  std::vector<std::pair<std::string, InputSpec*> > m_fields;
};

class ArgCVInputSpec : public InputSpec {
public:
  ArgCVInputSpec() {}
  virtual ~ArgCVInputSpec() {
    for (auto m : m_named_args) {
      if (m.second) delete m.second;
    }
    for (InputSpec *s : m_args) {
      if (s) delete s;
    }
  }
  void SetArgv0(std::string argv0) {
    m_argv0 = argv0;
  }
  void AddBool(std::string c, bool b) {
    m_bools.push_back({c, b});
  }
  void AddNamedArg(std::string c, InputSpec *spec) {
    m_named_args.push_back({c, spec});
  }
  void AddArg(InputSpec *spec) {
    m_args.push_back(spec);
  }

  virtual std::string GetRaw() override;
  virtual std::string GetSpec() override;
private:
  std::string m_argv0;
  std::vector<std::pair<std::string, bool> > m_bools;
  std::vector<std::pair<std::string, InputSpec*> > m_named_args;
  std::vector<InputSpec*> m_args;
};


#endif /* INPUT_SPEC_H */
