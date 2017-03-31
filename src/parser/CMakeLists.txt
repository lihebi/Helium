add_library(HeliumParser

  ast_v2.cc
  parser.cc
  source_manager.cc
  source_location.cc
  visitor.cpp
  Distributor.cpp
  Generator.cpp
  GrammarPatcher.cpp
  LevelVisitor.cpp
  ParentIndexer.cpp
  Printer.cpp
  TokenVisitor.cpp
  SymbolTableBuilder.cpp

  
  ast.cc
  ast_common.cc
  ast_cond.cc
  ast_iter.cc
  ast_node.cc
  ast_stmt.cc
  cfg.cc
  cond.cc
  decl.cc
  dom.cc
  gene.cc
  point_of_interest.cc
  resource.cc
  slice_reader.cc
  xml_doc_reader.cc
  xmlnode.cc
  xmlnode_helper.cc
  )
target_link_libraries(HeliumParser HeliumUtils HeliumResolver)
