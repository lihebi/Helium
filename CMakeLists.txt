cmake_minimum_required(VERSION 3.6)
project(helium)

set(CMAKE_CXX_STANDARD 11)

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