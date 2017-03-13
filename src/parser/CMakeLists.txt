add_library(HeliumParser

  # ast_v2.cc
  # parser.cc

  
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
