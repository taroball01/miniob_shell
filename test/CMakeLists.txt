# only cmake 3.24+ support boost 1.80+ so we got a warn
set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost 1.54 COMPONENTS unit_test_framework REQUIRED)

set(TEST_SOURCE_DIR ${PROJECT_SOURCE_DIR}/test)
set(TEST_MAIN_SRC ${TEST_SOURCE_DIR}/main.cpp)
# utils support
aux_source_directory(${TEST_SOURCE_DIR}/util TEST_UTIL_SOURCE_FILES)

# Test parser
aux_source_directory(${TEST_SOURCE_DIR}/sql TEST_PARSER_SOURCE_FILES)
add_executable(
  test_sql
  ${TEST_MAIN_SRC}
  ${TEST_PARSER_SOURCE_FILES}
  ${TEST_UTIL_SOURCE_FILES}
)

target_link_libraries(test_sql 
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
    Boost::unit_test_framework
)

add_test(
  NAME test_parser
  COMMAND $<TARGET_FILE:test_sql>
)

# Test relation support part
aux_source_directory(${TEST_SOURCE_DIR}/relation TEST_RELATION_SOURCE_FILES)
add_executable(
  test_relation
  ${TEST_MAIN_SRC}
  ${TEST_RELATION_SOURCE_FILES}
  ${TEST_UTIL_SOURCE_FILES}
)
target_link_libraries(test_relation 
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
    Boost::unit_test_framework
)
add_test(
  NAME test_relation
  COMMAND $<TARGET_FILE:test_relation>
)

# Test preprocessor
aux_source_directory(${TEST_SOURCE_DIR}/preprocess TEST_PREPROCESSOR_SOURCE_FILES)
add_executable(
  test_preprocessor
  ${TEST_MAIN_SRC}
  ${TEST_PREPROCESSOR_SOURCE_FILES}
  ${TEST_UTIL_SOURCE_FILES}
)
target_link_libraries(test_preprocessor
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
    Boost::unit_test_framework
)
add_test(
  NAME test_preprocessor
  COMMAND $<TARGET_FILE:test_preprocessor>
)

# test planner
aux_source_directory(${TEST_SOURCE_DIR}/planner TEST_PLANNER_SOURCE_FILES)
add_executable(
  test_planner
  ${TEST_MAIN_SRC}
  ${TEST_PLANNER_SOURCE_FILES}
  ${TEST_UTIL_SOURCE_FILES}
)

target_link_libraries(test_planner
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
    Boost::unit_test_framework
)
add_test(
  NAME test_planner
  COMMAND $<TARGET_FILE:test_planner>
)

# test optimizer
aux_source_directory(${TEST_SOURCE_DIR}/optimizer TEST_OPTIMIZER_SOURCE_FILES)
add_executable(
  test_optimizer
  ${TEST_MAIN_SRC}
  ${TEST_OPTIMIZER_SOURCE_FILES}
  ${TEST_UTIL_SOURCE_FILES}
)

target_link_libraries(test_optimizer
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
    Boost::unit_test_framework
)
add_test(
  NAME test_optimizer
  COMMAND $<TARGET_FILE:test_optimizer>
)

# test_executor
aux_source_directory(${TEST_SOURCE_DIR}/executor TEST_EXECUTOR_SOURCE_FILES) 
add_executable(
  test_executor
  ${TEST_MAIN_SRC}
  ${TEST_EXECUTOR_SOURCE_FILES}
  ${TEST_UTIL_SOURCE_FILES}
)
target_link_libraries(test_executor
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
    Boost::unit_test_framework
)
add_test(
  NAME test_executor
  COMMAND $<TARGET_FILE:test_executor>
)