add_executable(helium-ast-printer
  ast-printer.cc)
target_link_libraries(helium-ast-printer
  HeliumUtils
  HeliumParser
  HeliumResolver
  HeliumType
  HeliumWorkflow)
target_link_libraries(helium-ast-printer
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES})

target_link_libraries(helium-ast-printer pugixml sqlite3)





add_executable(helium-cfg-printer
  cfg-printer.cc)
target_link_libraries(helium-cfg-printer
  HeliumUtils
  HeliumParser
  HeliumResolver
  HeliumType
  HeliumWorkflow)
target_link_libraries(helium-cfg-printer
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES})

target_link_libraries(helium-cfg-printer pugixml sqlite3)


# add_test (NAME ast-printer-test COMMAND helium-ast-printer ../../test/cfgtest/if.c)
# add_custom_target(NAME ast-printer-test COMMAND helium-ast-printer ../../test/cfgtest/if.c)
# execute_process(COMMAND ./bin/helium-ast-printer ../../test/cfgtest/if.c)
# add_custom_command(TARGET helium-ast-printer POST_BUILD
#   COMMAND echo "Finished Compiling, Running test .."
#   COMMAND echo ""
#   COMMAND helium-ast-printer ~/github/helium/test/cfgtest/if.c || echo "failed" && echo "")
