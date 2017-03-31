add_subdirectory(parser)
add_subdirectory(resolver)
add_subdirectory(type)
add_subdirectory(utils)
add_subdirectory(workflow)

add_executable(helium
  main.cc
  # clangSnippet.cpp
  )
target_link_libraries(helium
        HeliumUtils
        HeliumParser
        HeliumResolver
        HeliumType
        HeliumWorkflow)
target_link_libraries(helium
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LIBRARIES}
        ${GTEST_BOTH_LIBRARIES})

target_link_libraries(helium pugixml sqlite3 ctags)
