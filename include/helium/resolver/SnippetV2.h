#ifndef SNIPPETV2_H
#define SNIPPETV2_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <assert.h>
#include <readtags.h>
#include <sstream>

#include "helium/parser/source_location.h"
#include "helium/utils/fs_utils.h"


#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

namespace v2 {

  // level 1
  class FunctionSnippet;
  class TypedefSnippet;
  class VarSnippet;
  class RecordSnippet;
  class EnumSnippet;
  /**
   * Snippet
   *
   * This can be created from
   * - SnippetFactory, using tagfile and code
   * - Load From Database
   *
   * Some characteristics:
   * - Every source piece of code has exactly one snippet instance.
   * - Lexical different snippet might semantically refer to the same one.
   * - Snippets can have relationship:
   *   - function decl vs. function definition
   *   - structure decl vs. typedef
   *   - structure decl vs. variable decl
   *
   * Code is not read by default, because it is a virtual function, which should not be called in constructor.
   * I plan to lazily read code when getCode is called.
   */
  class Snippet {
  public:
    Snippet() {}
    Snippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Name(name), File(file), Begin(begin), End(end) {
    }
    virtual ~Snippet() {}
    std::string getCode() {
      if (Code.empty()) readCode();
      // FIXME will this fail often?
      assert(!Code.empty());
      return Code;
    }
    Snippet *getCannonicalSnippet();

    bool isTypedef();
    bool isFunctionDefinition();
    bool isFunctionDeclaration();
    bool isRecord();
    bool isVariable();
    RecordSnippet *getAsRecordSnippet();
    std::string getName() {return Name;}
    virtual std::string getSnippetName() = 0;
    virtual void save(std::ostream &os) {
      os << getSnippetName() << " " << ID << " " << Name << " " << File << " "
         << Begin.getLine() << " " << Begin.getColumn() << " "
         << End.getLine() << " " << End.getColumn() << "\n";
    }
    virtual void load(std::istream &is) {
      is >> ID >> Name >> File >> Begin >> End;
    }
    void setId(int id) {ID = id;}
    int getId() {return ID;}
    virtual void dump(std::ostream &os) = 0;

    // void loadCode(fs::path p) {
    //   Code = utils::read_file(p.string());
    // }
    virtual void readCode();
  protected:
    int ID;
    std::string Name;
    std::string File;
    SourceLocation Begin;
    SourceLocation End;
    std::string Code;
  };

  /**
   * Function
   */
  class FunctionSnippet : public Snippet {
  public:
    FunctionSnippet() {}
    FunctionSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {
    }
    virtual ~FunctionSnippet() {}
    virtual std::string getSnippetName() {return "FunctionSnippet";}
    virtual void save(std::ostream &os) {
      Snippet::save(os);
    }
    virtual void load(std::istream &is) {
      Snippet::load(is);
    }
    virtual void dump(std::ostream &os) {
      os << "FucntionSnippet " << Name;
    }
  private:
  };

  class VarSnippet : public Snippet {
  public:
    VarSnippet() {}
    VarSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {}
    virtual std::string getSnippetName() {return "VarSnippet";}
    virtual ~VarSnippet() {}
    virtual void save(std::ostream &os) {
      Snippet::save(os);
    }
    virtual void load(std::istream &is) {
      Snippet::load(is);
    }
    virtual void dump(std::ostream &os) {
      os << "VarSnippet " << Name;
    }
    virtual void readCode();
  private:
  };

  class TypedefSnippet : public Snippet {
  public:
    TypedefSnippet() {}
    TypedefSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {
    }
    virtual ~TypedefSnippet() {}
    virtual std::string getSnippetName() {return "TypedefSnippet";}
    virtual void save(std::ostream &os) {
      Snippet::save(os);
    }
    virtual void load(std::istream &is) {
      Snippet::load(is);
    }
    virtual void dump(std::ostream &os) {
      os << "TypedefSnippet " << Name;
    }
  private:
  };

  /**
   * union, struct
   */
  class RecordSnippet : public Snippet {
  public:
    RecordSnippet() {}
    RecordSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {
    }
    virtual ~RecordSnippet() {}
    virtual std::string getSnippetName() {return "RecordSnippet";}
    virtual void save(std::ostream &os) {
      os << getSnippetName() << " " << ID << " " << Name << " " << File << " "
         << Begin.getLine() << " " << Begin.getColumn() << " "
         << End.getLine() << " " << End.getColumn() << "\n";
    }
    virtual void load(std::istream &is) {
      is >> ID >> Name >> Begin >> End;
    }
    virtual void dump(std::ostream &os) {
      os << "RecordSnippet " << Name;
    }
  private:
  };
  
  class EnumSnippet : public Snippet {
  public:
    EnumSnippet() {}
    EnumSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {
    }
    virtual std::string getSnippetName() {return "EnumSnippet";}
    virtual ~EnumSnippet() {}
    virtual void save(std::ostream &os) {
      Snippet::save(os);
      // members
      os << "    ";
      for (auto &s : Fields) {
        os << s << " ";
      }
      os << "\n";
    }
    virtual void load(std::istream &is) {
      Snippet::load(is);
      std::string line;
      getline(is, line);
      std::istringstream iss(line);
      while (iss >> line) {
        addField(line);
      }
    }
    void addField(std::string field) {
      Fields.push_back(field);
    }
    virtual void dump(std::ostream &os) {
      os << "EnumSnippet " << Name;
    }
  private:
    std::vector<std::string> Fields;
  };

  // a type will have a pointer to its canonical type
  // a canonical type is a primitive type or the pointer to primitive type
  // a user-defined type will have pointer to its snippet
  // typedef struct A A.
  // there will be:
  // - type A ==> Snippet
  // - type struct A ==> Snippet
  // - type A* ==> Snippet


  /**
   * The snippet manager should maintain:
   * - dependence
   * - relation
   *   - same type
   *   - function def, decl
   *   - call graph
   */
  class SnippetManager {
  public:
    SnippetManager() {}
    ~SnippetManager() {}
    void add(Snippet *s) {
      s->setId(Snippets.size());
      Snippets.push_back(s);
    }
    void add(std::vector<Snippet*> snippets) {
      for (Snippet *s : snippets) {
        add(s);
      }
    }

    /**
     * create index
     * so that snippets can be queired by name
     */
    void process() {
      for (Snippet *s : Snippets) {
        std::string name = s->getName();
        KeyMap[name].push_back(s);
      }
    }

    std::vector<Snippet*> get(std::string key) {
      if (KeyMap.count(key)==1) return KeyMap[key];
      return {};
    }
    Snippet *get(std::string key, std::string kind) {
      if (KeyMap.count(key)==1) {
        for (Snippet *s : KeyMap[key]) {
          if (s->getSnippetName() == kind) return s;
        }
      }
      return nullptr;
    }
    /**
     * create Deps and IdDeps
     */
    void createDep() {}
    /**
     * save to disk
     *
     * {ID: 1, kind: EnumSnippet, 
     * }
     */
    void saveSnippet(fs::path p) {
      std::ofstream os;
      os.open(p.string());
      assert(os.is_open());
      for (Snippet *s : Snippets) {
        s->save(os);
      }
    }

    /**
     * save code to disk
     * FIXME Do I need to do this??
     * NO I don't need this.
     */
    // void saveCode(fs::path dir) {}

    void saveDeps(fs::path p) {
      std::ofstream os;
      os.open(p.string());
      assert(os.is_open());
      for (auto &m : Deps) {
        os << m.first->getId() << " ";
        for (Snippet *s : m.second) {
          os << s->getId() << " ";
        }
        os << "\n";
      }
    }
    /**
     * load from disk
     */
    void loadSnippet(fs::path p) {
      assert(Snippets.empty());
      std::ifstream is;
      is.open(p.string());
      assert(is.is_open());
      std::string kind;
      Snippet *s = nullptr;
      while (is >> kind) {
        if (kind == "FunctionSnippet") s = new FunctionSnippet();
        else if (kind == "VarSnippet") s = new VarSnippet();
        else if (kind == "TypedefSnippet") s = new TypedefSnippet();
        else if (kind == "RecordSnippet") s = new RecordSnippet();
        else {
          assert(false);
        }
        s->load(is);
        int id = Snippets.size();
        s->setId(id);
        Snippets.push_back(s);
      }
    }
    /**
     * Load dependence
     */
    void loadDependence(fs::path p) {
      std::ifstream is;
      is.open(p.string());
      assert(is.is_open());
      std::string line;
      while (getline(is, line)) {
        int from_id,to_id;
        std::istringstream ss(line);
        ss >> from_id;
        while (ss >> to_id) {
          // IdDeps[from_id].insert(to_id);
          Deps[Snippets[from_id]].insert(Snippets[to_id]);
        }
      }
    }

    void dump(std::ostream &os) {
      os << "total " << Snippets.size() << " snippets\n";
      for (Snippet *s : Snippets) {
        // s->dump(os);
        // os << "\n";
        s->save(os);
        os << s->getCode();
        os << "\n";
      }
    }
    /**
     * check whether the two snippet manager maintain equivalent thing
     * It is used to verify whether the save and load get the same thing
     * It will not check pointer address.
     * Instead, it check the values.
     */
    // bool equivalent(SnippetManager *rhs) {
      
    // }

    std::vector<Snippet*> getSnippets() {
      return Snippets;
    }
  private:
    std::vector<Snippet*> Snippets; // the index is the ID
    std::map<Snippet*, std::set<Snippet*> > Deps;
    std::map<std::string, std::vector<Snippet*> > KeyMap;
    // std::map<int, std::set<int> > IdDeps;
  };
  
}

#endif /* SNIPPETV2_H */
