add_library(HeliumType
  ClangSnippet.cpp
  Cache.cpp
  Snippet.cpp
  SnippetAction.cpp)
target_link_libraries(HeliumType HeliumUtils)
