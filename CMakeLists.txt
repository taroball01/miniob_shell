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

# set directory alias
set(MAIN_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(STANDALONE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/standalone)
set(TEST_SOURCE_DIR ${PROJECT_SOURCE_DIR}/test)
set(INCLUDE_HEADER_DIR ${PROJECT_SOURCE_DIR}/include)
# include .h directories  
include_directories(${INCLUDE_HEADER_DIR})

# main src
set(QUERY_PROCESS_ENGINE_LIB query_process_engine)
# parser dependencies 
# see: https://cmake.org/cmake/help/latest/module/FindFLEX.html
find_package(FLEX 2.6.4)
find_package(BISON 2.8.0)

set(PARSER_PRIMARY_DIR ${PROJECT_SOURCE_DIR}/include/sql)
set(PARSER_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/sql)

set(YACC_SOURCE_FILE ${PARSER_SOURCE_DIR}/yacc.tab.cpp)
set(FLEX_SOURCE_FILE ${PARSER_SOURCE_DIR}/lexer.yy.cpp)

BISON_TARGET(
  Parser ${PARSER_PRIMARY_DIR}/yacc.y ${YACC_SOURCE_FILE}
  DEFINES_FILE ${PARSER_PRIMARY_DIR}/yacc.tab.h
)
FLEX_TARGET(
  Lexer ${PARSER_PRIMARY_DIR}/lexer.l  ${FLEX_SOURCE_FILE}
  DEFINES_FILE ${PARSER_PRIMARY_DIR}/lexer.yy.h
)

set_source_files_properties(
  ${YACC_SOURCE_FILE}
  ${FLEX_SOURCE_FILE}
  PROPERTIES
  LANGUAGE CXX
  COMPILE_FLAGS -Wno-unused-function
)

set(PARSER_SOURCE_FILES 
  ${YACC_SOURCE_FILE}
  ${FLEX_SOURCE_FILE}
  ${PROJECT_SOURCE_DIR}/src/sql/parser.cpp
)
set(QUERY_PROCESS_ENGINE_SOURCE_FILES ${PARSER_SOURCE_FILES})
aux_source_directory(${MAIN_SOURCE_DIR}/executor QUERY_PROCESS_ENGINE_SOURCE_FILES)
aux_source_directory(${MAIN_SOURCE_DIR}/optimizer QUERY_PROCESS_ENGINE_SOURCE_FILES)
aux_source_directory(${MAIN_SOURCE_DIR}/planner QUERY_PROCESS_ENGINE_SOURCE_FILES)

aux_source_directory(${MAIN_SOURCE_DIR}/preprocess QUERY_PROCESS_ENGINE_SOURCE_FILES)
aux_source_directory(${MAIN_SOURCE_DIR}/preprocess/stmt QUERY_PROCESS_ENGINE_SOURCE_FILES)

aux_source_directory(${MAIN_SOURCE_DIR}/relation QUERY_PROCESS_ENGINE_SOURCE_FILES)
aux_source_directory(${MAIN_SOURCE_DIR}/relation/value QUERY_PROCESS_ENGINE_SOURCE_FILES)
# make sure this call after add_subdirectory(sql)
add_library(${QUERY_PROCESS_ENGINE_LIB} STATIC  ${QUERY_PROCESS_ENGINE_SOURCE_FILES})


# clean genrated flex and bison file
add_custom_target(clean_parser 
  COMMAND ${CMAKE_COMMAND} -E remove ${BISON_Parser_OUTPUTS}
  COMMAND ${CMAKE_COMMAND} -E remove ${FLEX_Lexer_OUTPUTS}
)

# test 
enable_testing()
add_subdirectory(${TEST_SOURCE_DIR})

# standalone
add_subdirectory(${STANDALONE_SOURCE_DIR})

# clang format
find_program(CLANG_FORMAT_BIN NAMES clang-format)

if ("${CLANG_FORMAT_BIN}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
    message(WARNING "couldn't find clang-format.")
else ()
    message(STATUS "found clang-format at ${CLANG_FORMAT_BIN}")
endif ()

add_custom_target(format 
  COMMAND ${CLANG_FORMAT_BIN} -i 
    ${PROJECT_SOURCE_DIR}/src/**/*.cpp 
    ${PROJECT_SOURCE_DIR}/include/**/*.h
    ${PROJECT_SOURCE_DIR}/test/**/*
)
