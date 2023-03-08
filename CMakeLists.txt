cmake_minimum_required(VERSION 3.17)
project(miniob_shell VERSION 1.0.0 LANGUAGES CXX)

# set compile options
add_compile_options(-Wall -Werror)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Compile options for Debug variant
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -g -O0")

# Compile options for Release variant
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# directories  
include_directories(src)

# parser part 
# see: https://cmake.org/cmake/help/latest/module/FindFLEX.html
find_package(FLEX 2.6.4)
find_package(BISON 2.8.0)

set(YACC_SOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/yacc.tab.c)
set(FLEX_SOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.c)
BISON_TARGET(
  Parser src/sql/yacc.y ${YACC_SOURCE_FILE}
  DEFINES_FILE ${PROJECT_SOURCE_DIR}/src/sql/yacc.tab.h
)
FLEX_TARGET(Lexer src/sql/lexer.l  ${FLEX_SOURCE_FILE}
  DEFINES_FILE ${PROJECT_SOURCE_DIR}/src/sql/lexer.yy.h
)

set_source_files_properties(
  ${YACC_SOURCE_FILE}
  ${FLEX_SOURCE_FILE}
  PROPERTIES
  LANGUAGE CXX
  COMPILE_FLAGS -Wno-unused-function
)

add_library(parser STATIC  ${YACC_SOURCE_FILE} ${FLEX_SOURCE_FILE})

add_executable(shell
  src/shell.cpp
  src/util.cpp
)

target_link_libraries(shell parser)

# test 
# only cmake 3.24+ support boost 1.80+ so we got a warn
set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost 1.54 COMPONENTS unit_test_framework REQUIRED)

set(TEST_SOURCE_DIR ${PROJECT_SOURCE_DIR}/test)
set(TEST_MAIN_SRC ${TEST_SOURCE_DIR}/main.cpp)

add_executable(
  test_sql
  ${TEST_MAIN_SRC}
  ${TEST_SOURCE_DIR}/sql/test_trivial_sql.cpp
)
target_link_libraries(test_sql
  PRIVATE
    parser
    Boost::unit_test_framework
)

enable_testing()
add_test(
  NAME test_parser
  COMMAND $<TARGET_FILE:test_sql>
)