cmake_minimum_required(VERSION 3.6)
project(helium)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-O0")
# set(CMAKE_CXX_FLAGS "-pg -g")

# https://github.com/ninja-build/ninja/wiki/FAQ
# ninja runs in parallel, so buffer the output for each thread
# clang would detect that its output is not directed to a terminal
# thus do not print the color
# so force it to use color
# Add_compile_options(-fcolor-diagnostics)
Add_compile_options(-g)

# remove this to avoid profiling overhead
# Add_compile_options(-pg)
# Add_compile_options(-lprofiler)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package (Threads)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

find_package( Boost 1.40 COMPONENTS system filesystem program_options regex timer REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )



include_directories(include)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)

find_package(Doxygen)
if((DOXYGEN_FOUND))
  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${doxyfile_in} ${doxyfile} @ONLY)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
endif()
  
