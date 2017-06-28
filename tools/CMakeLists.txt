add_executable(helium-cfg-printer
  cfg-printer.cc)
target_link_libraries(helium-cfg-printer
  HeliumUtils
  HeliumType
  HeliumParser
  )

target_link_libraries(helium-cfg-printer
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_LIBRARIES}
  ${GTEST_BOTH_LIBRARIES})

target_link_libraries(helium-cfg-printer pugixml sqlite3)
