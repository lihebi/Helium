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
    virtual void readCode() = 0;
    std::string getFile() {return File;}
    SourceLocation getBeginLoc() {return Begin;}
    SourceLocation getEndLoc() {return End;}
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
    virtual void readCode();
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
    virtual void readCode();
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
    virtual void readCode();
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
    virtual void readCode();
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
     * traverse dir and process all source files
     */
    void traverseDir(fs::path dir);

    /**
     * create index
     * so that snippets can be queired by name
     */
    void process() {
      for (Snippet *s : Snippets) {
        std::string name = s->getName();
        KeyMap[name].push_back(s);
      }
      createDeps();
      createOuters();
    }

    /**
     * create Deps and IdDeps
     */
    void createDeps();
    void createOuters();
    /**
     * save to disk
     */
    void saveSnippet(fs::path p);
    void saveDeps(fs::path p);
    void saveOuters(fs::path p);
    /**
     * load from disk
     */
    void loadSnippet(fs::path p);
    void loadDeps(fs::path p);
    void loadOuters(fs::path p);

    /**
     * dump only the size of snippets, 
     */
    void dumpLight(std::ostream &os);
    void dump(std::ostream &os);
    void dumpSnippetsVerbose(std::ostream &os);
    /**
     * Getters
     */
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
    std::set<Snippet*> getDep(Snippet* s) {
      if (Deps.count(s) == 1) return Deps[s];
      return {};
    }
    std::set<Snippet*> getOuter(Snippet *s) {
      if (Outers.count(s) == 1) return Outers[s];
      return {};
    }
    /**
     * Recursively get all deps
     */
    std::set<Snippet*> getAllDep(Snippet *s);
    /**
     * Simple Getters
     */
    std::vector<Snippet*> getSnippets() {return Snippets;}
    std::map<Snippet*, std::set<Snippet*> > getDeps() {return Deps;}
    std::map<Snippet*, std::set<Snippet*> > getOuters() {return Outers;}

    /**
     * for testing
     */
    std::map<int, std::set<int> > getDepsAsId() {
      std::map<int, std::set<int> > ret;
      for (auto &m : Deps) {
        int from = m.first->getId();
        for (Snippet *s : m.second) {
          ret[from].insert(s->getId());
        }
      }
      return ret;
    }
    std::map<int, std::set<int> > getOutersAsId() {
      std::map<int, std::set<int> > ret;
      for (auto &m : Outers) {
        int from = m.first->getId();
        for (Snippet *s : m.second) {
          ret[from].insert(s->getId());
        }
      }
      return ret;
    }
  private:
    std::vector<Snippet*> Snippets; // the index is the ID
    std::map<Snippet*, std::set<Snippet*> > Deps;
    std::map<Snippet*, std::set<Snippet*> > Outers;
    std::map<std::string, std::vector<Snippet*> > KeyMap;
    // std::map<int, std::set<int> > IdDeps;
  };
  
}

#endif /* SNIPPETV2_H */
