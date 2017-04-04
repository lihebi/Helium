add_executable(heliumtest
  test_parser.cpp
  test_snippet.cpp
  test_generator.cpp
  test_graph.cpp
  test_SourceManager.cpp
  test_main.cc)

target_link_libraries(heliumtest
        HeliumUtils
        HeliumParser
        HeliumResolver
        HeliumType
        HeliumWorkflow)

target_link_libraries(heliumtest
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LIBRARIES}
        ${GTEST_BOTH_LIBRARIES})

target_link_libraries(heliumtest pugixml sqlite3 ctags)
