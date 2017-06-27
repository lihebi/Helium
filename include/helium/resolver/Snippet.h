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

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace fs = boost::filesystem;

namespace v2 {

  // level 1
  class FunctionSnippet;
  class TypedefSnippet;
  class VarSnippet;
  class RecordSnippet;
  class EnumSnippet;

  class FunctionDeclSnippet;
  class RecordDeclSnippet;
  
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
    virtual std::set<std::string> getKeys() {
      return {Name};
    }

    bool isTypedef();
    bool isFunctionDefinition();
    bool isFunctionDeclaration();
    bool isRecord();
    bool isVariable();
    RecordSnippet *getAsRecordSnippet();
    std::string getName() {return Name;}
    virtual std::string getSnippetName() = 0;
    // virtual void save(std::ostream &os) {
    //   os << getSnippetName() << " " << ID << " " << Name << " " << File << " "
    //      << Begin.getLine() << " " << Begin.getColumn() << " "
    //      << End.getLine() << " " << End.getColumn() << "\n";
    // }
    // virtual void load(std::istream &is) {
    //   is >> ID >> Name >> File >> Begin >> End;
    // }
    void setId(int id) {ID = id;}
    int getId() {return ID;}
    virtual void dump(std::ostream &os);
    virtual void dumpVerbose(std::ostream &os);
    std::string toString() {
      return getSnippetName() + " at " + File + ":"
        + std::to_string(Begin.getLine());
    }

    // void loadCode(fs::path p) {
    //   Code = utils::read_file(p.string());
    // }
    virtual void readCode() = 0;
    std::string getFile() {return File;}
    SourceLocation getBeginLoc() {return Begin;}
    SourceLocation getEndLoc() {return End;}

    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator);
    virtual void loadJson(rapidjson::Value &v);

    // void addDep(int dep) {Deps.insert(dep);}
    // void addDep(std::set<int> deps) {Deps.insert(deps.begin(), deps.end());}
    // void addDep(std::vector<int> deps) {Deps.insert(deps.begin(), deps.end());}
    void addDep(Snippet *s) {
      if (s) {Deps.insert(s);}
    }
    void addDep(std::vector<Snippet *> deps) {
      Deps.insert(deps.begin(), deps.end());
    }
    void removeDep(Snippet *s) {
      if (s) {
        Deps.erase(s);
      }
    }
    std::set<Snippet*> getDeps() {return Deps;}
    std::set<int> getDepsAsId() {
      std::set<int> ret;
      for (auto *s : Deps) {
        ret.insert(s->getId());
      }
      return ret;
    }
    std::set<Snippet*> getAllDeps();
    std::set<Snippet*> getOuters() {return Outers;}
    bool isOuter(Snippet *s)  {return Outers.count(s) == 1;}
    std::set<int> getOutersAsId() {
      std::set<int> ret;
      for (auto *s : Outers) {
        ret.insert(s->getId());
      }
      return ret;
    }
    void addOuter(Snippet *s) {
      if (s) Outers.insert(s);
    }
  protected:
    int ID;
    std::string Name;
    std::string File;
    SourceLocation Begin;
    SourceLocation End;
    std::string Code;
    // snippet ID this one depends on
    std::set<Snippet*> Deps;
    // snippt ID that encloses this one
    std::set<Snippet*> Outers;
  };

  /**
   * Function
   */
  class FunctionSnippet : public Snippet {
  public:
    FunctionSnippet() {}
    FunctionSnippet(std::string name, std::string file,
                    SourceLocation begin, SourceLocation end,
                    SourceLocation body_begin)
      : Snippet(name, file, begin, end), body_begin(body_begin) {
    }
    virtual ~FunctionSnippet() {}
    virtual std::string getSnippetName() {return "FunctionSnippet";}
    virtual void readCode();
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator) {
      rapidjson::Value v = Snippet::saveJson(allocator);
      v.AddMember("kind", "FunctionSnippet", allocator);
      return v;
    }
    virtual void loadJson(rapidjson::Value &v) {
      Snippet::loadJson(v);
    }
    /**
     * Declaration of function.
     */
    std::string getFuncDecl();
  private:
    // seems that this is not used
    SourceLocation body_begin;
  };

  class FunctionDeclSnippet : public Snippet {
  public:
    FunctionDeclSnippet() {}
    FunctionDeclSnippet(std::string name, std::string file,
                        SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {}
    virtual ~FunctionDeclSnippet() {}
    virtual void readCode();
    virtual std::string getSnippetName() {return "FunctionDeclSnippet";}
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator) {
      rapidjson::Value v = Snippet::saveJson(allocator);
      v.AddMember("kind", "FunctionDeclSnippet", allocator);
      return v;
    }
    virtual void loadJson(rapidjson::Value &v) {
      Snippet::loadJson(v);
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
    // virtual void save(std::ostream &os) {
    //   Snippet::save(os);
    // }
    // virtual void load(std::istream &is) {
    //   Snippet::load(is);
    // }
    // virtual void dump(std::ostream &os) {
    //   os << "VarSnippet " << Name;
    // }
    virtual void readCode();
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator) {
      rapidjson::Value v = Snippet::saveJson(allocator);
      v.AddMember("kind", "VarSnippet", allocator);
      return v;
    }
    virtual void loadJson(rapidjson::Value &v) {
      Snippet::loadJson(v);
    }
  private:
  };

  /**
   * Note that a Typedef may also enclose a typedef.
   * typedef A b,*c;
   * there will be two typedefs, b and c
   */
  class TypedefSnippet : public Snippet {
  public:
    TypedefSnippet() {}
    TypedefSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {
    }
    virtual ~TypedefSnippet() {}
    virtual std::string getSnippetName() {return "TypedefSnippet";}
    // virtual void save(std::ostream &os) {
    //   Snippet::save(os);
    // }
    // virtual void load(std::istream &is) {
    //   Snippet::load(is);
    // }
    // virtual void dump(std::ostream &os) {
    //   os << "TypedefSnippet " << Name;
    // }
    virtual void readCode();
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator) {
      rapidjson::Value v = Snippet::saveJson(allocator);
      v.AddMember("kind", "TypedefSnippet", allocator);
      return v;
    }
    virtual void loadJson(rapidjson::Value &v) {
      Snippet::loadJson(v);
    }
    /**
     * Get decl of it.
     * typedef struct A {} X,*Y;
     * - typedef struct A X,*Y;
     * typedef struct B X; => output itself
     * typedef struct {} xxx; => NOTHING!
     */
    std::string getDecl();
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
    // virtual void save(std::ostream &os) {
    //   os << getSnippetName() << " " << ID << " " << Name << " " << File << " "
    //      << Begin.getLine() << " " << Begin.getColumn() << " "
    //      << End.getLine() << " " << End.getColumn() << "\n";
    // }
    // virtual void load(std::istream &is) {
    //   Snippet::load(is);
    // }
    // virtual void dump(std::ostream &os) {
    //   os << "RecordSnippet " << Name;
    // }
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator) {
      rapidjson::Value v = Snippet::saveJson(allocator);
      v.AddMember("kind", "RecordSnippet", allocator);
      return v;
    }
    virtual void loadJson(rapidjson::Value &v) {
      Snippet::loadJson(v);
    }
    virtual void readCode();
    /**
     * struct A {} => struct A
     */
    std::string getDecl();
  private:
  };

  class RecordDeclSnippet : public Snippet {
  public:
    RecordDeclSnippet() {}
    RecordDeclSnippet(std::string name, std::string file,
                               SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {}
    virtual ~RecordDeclSnippet() {}
    virtual std::string getSnippetName() {return "RecordDeclSnippet";}
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator) {
      rapidjson::Value v = Snippet::saveJson(allocator);
      v.AddMember("kind", "RecordDeclSnippet", allocator);
      return v;
    }
    virtual void loadJson(rapidjson::Value &v) {
      Snippet::loadJson(v);
    }
    virtual void readCode();
  };
  
  class EnumSnippet : public Snippet {
  public:
    EnumSnippet() {}
    EnumSnippet(std::string name, std::string file, SourceLocation begin, SourceLocation end)
      : Snippet(name, file, begin, end) {
    }
    virtual std::string getSnippetName() {return "EnumSnippet";}
    virtual ~EnumSnippet() {}
    // virtual void save(std::ostream &os) {
    //   Snippet::save(os);
    //   // members
    //   os << "    ";
    //   for (auto &s : Fields) {
    //     os << s << " ";
    //   }
    //   os << "\n";
    // }
    // virtual void load(std::istream &is) {
    //   Snippet::load(is);
    //   std::string line;
    //   getline(is, line);
    //   std::istringstream iss(line);
    //   while (iss >> line) {
    //     addField(line);
    //   }
    // }
    void addField(std::string field) {
      Fields.push_back(field);
    }
    // virtual void dump(std::ostream &os) {
    //   os << "EnumSnippet " << Name;
    // }
    virtual void readCode();
    virtual rapidjson::Value saveJson(rapidjson::Document::AllocatorType &allocator);
    virtual void loadJson(rapidjson::Value &v);
    virtual std::set<std::string> getKeys() {
      std::set<std::string> ret;
      ret.insert(Name);
      ret.insert(Fields.begin(), Fields.end());
      return ret;
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
    // FIXME Instance()??? make the constructor private
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
        // std::string name = s->getName();
        // KeyMap[name].push_back(s);
        std::set<std::string> keys = s->getKeys();
        for (std::string key : keys) {
          KeyMap[key].push_back(s);
        }
      }
      createDeps();
      createOuters();
      sortFiles();
    }

    /**
     * FIXME CAUTION must call this after load!!!
     */
    void processAfterLoad() {
      for (Snippet *s : Snippets) {
        // std::string name = s->getName();
        // KeyMap[name].push_back(s);
        std::set<std::string> keys = s->getKeys();
        for (std::string key : keys) {
          KeyMap[key].push_back(s);
        }
      }
      sortFiles();
    }

    /**
     * Sort files based on Topological sorting
     * Considering only type snippets: Record, Enum, Typedef
     */
    void sortFiles();
    void topoSortFiles();

    /**
     * create Deps and IdDeps
     */
    void createDeps();
    void createOuters();

    // void save(fs::path p) {
    //   saveSnippet(p / "snippets.txt");
    //   saveDeps(p / "deps.txt");
    //   saveOuters(p / "outers.txt");
    // }
    // void load(fs::path p) {
    //   loadSnippet(p / "snippets.txt");
    //   loadDeps(p / "deps.txt");
    //   loadOuters(p / "outers.txt");
    // }

    void saveJson(fs::path p);
    void loadJson(fs::path p);
  public:

    void dump(std::ostream &os);
    void dumpVerbose(std::ostream &os);
    /**
     * return the information, with each line prefixed by //
     */
    std::string dumpComment();
    /**
     * Getters
     */
    std::vector<Snippet*> get(std::string key) {
      if (KeyMap.count(key)==1) return KeyMap[key];
      return {};
    }
    // FIXME get with same name might have multiple
    // e.g. function decl
    // But maybe this function only appear in the test file?
    Snippet *getone(std::string key, std::string kind) {
      if (KeyMap.count(key)==1) {
        for (Snippet *s : KeyMap[key]) {
          if (s->getSnippetName() == kind) return s;
        }
      }
      return nullptr;
    }
    std::vector<Snippet*> get(std::string key, std::string kind) {
      std::vector<Snippet*> ret;
      for (Snippet *s : KeyMap[key]) {
        if (s->getSnippetName() == kind) ret.push_back(s);
      }
      return ret;
    }
    // std::set<Snippet*> getDep(Snippet* s) {
    //   if (Deps.count(s) == 1) return Deps[s];
    //   return {};
    // }
    // std::set<Snippet*> getOuter(Snippet *s) {
    //   if (Outers.count(s) == 1) return Outers[s];
    //   return {};
    // }
    /**
     * Recursively get all deps
     */
    std::set<Snippet*> getAllDep(Snippet *s);
    /**
     * if there's a outer of any of them, use that outer
     */
    std::set<Snippet*> replaceNonOuters(std::set<Snippet*> ss);
    
    /**
     * Simple Getters
     */
    std::vector<Snippet*> getSnippets() {return Snippets;}

    // - get only the outers
    // - order by file
    // - sort by line number
    // - get all .h files
    // - use headersorter to sort all .h files
    //   - load all header files in ORIGINAL source folder
    //   - locate #include ..., and insert information
    // - put all .c files at the end

    std::vector<Snippet*> sort(std::set<Snippet*> snippets);

    int size() {return Snippets.size();}
    std::set<v2::Snippet*> getAllDeps(std::set<v2::Snippet*> snippets);

    bool checkValid(std::string &reason);
  private:
    std::vector<Snippet*> Snippets; // the index is the ID
    // std::map<Snippet*, std::set<Snippet*> > Deps;
    // std::map<Snippet*, std::set<Snippet*> > Outers;
    // generated
    std::map<std::string, std::vector<Snippet*> > KeyMap;
    std::map<std::string, std::set<Snippet*> > FileMap;
    // std::map<std::string, std::set<std::string> > FileDep;
    // sorted files
    std::vector<std::string> FileV;
    // sorted snippets
    std::vector<Snippet*> SnippetV;
    // std::map<int, std::set<int> > IdDeps;
  };


  class GlobalSnippetManager : public SnippetManager {
  public:
    static GlobalSnippetManager *Instance() {
      if (!instance) {
        instance = new GlobalSnippetManager();
      }
      return instance;
    }
  private:
    GlobalSnippetManager() {}
    ~GlobalSnippetManager() {}
    static GlobalSnippetManager *instance;
  };
  
}

#endif /* SNIPPETV2_H */
