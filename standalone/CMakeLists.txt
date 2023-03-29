aux_source_directory(${STANDALONE_SOURCE_DIR} STANDALONE_SOURCE_FILES)

add_executable(shell 
  ${STANDALONE_SOURCE_FILES}
)

target_link_libraries(shell
  PRIVATE
    ${QUERY_PROCESS_ENGINE_LIB}
)