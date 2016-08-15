#ifndef INPUT_SPEC_H
#define INPUT_SPEC_H

#include "common.h"

/**
 * The spec for a chain of type is hard. But I have solution.
 * The spec format is a like-"json" format: use {} for the level.
 */
class InputSpec {
public:
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
  virtual std::string GetSpec() override;

  virtual std::string GetRaw() override;
  
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
    if (m_to) {
      delete m_to;
    }
  }
  virtual std::string GetRaw() override {
    if (m_to) {
      return m_to->GetRaw();
    }
    return "";
  }

  virtual std::string GetSpec() override;

  virtual void Add(InputSpec *spec) override {
    if (m_to) delete m_to;
    m_to = spec;
  }

private:
  InputSpec *m_to;
  
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
