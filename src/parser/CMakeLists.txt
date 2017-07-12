add_library(HeliumParser
  AST.cpp
  Parser.cpp
  SourceManager.cpp
  SourceLocation.cpp
  IncludeManager.cpp
  LibraryManager.cpp
  
  Visitor.cpp
  CFGBuilder.cpp
  Distributor.cpp
  Generator.cpp
  GrammarPatcher.cpp
  LevelVisitor.cpp
  ParentIndexer.cpp
  Printer.cpp
  SymbolTableBuilder.cpp
  TokenVisitor.cpp
  )
target_link_libraries(HeliumParser HeliumUtils HeliumType)
