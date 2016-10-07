find_package(Doxygen)

if(DOXYGEN_FOUND)

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dox)

  configure_file(${CMAKE_SOURCE_DIR}/cmake/toconfigure/GiBUUTools.dox.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/dox/GiBUUTools.dox.cfg @ONLY)
  add_custom_target(doc_generate
  ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/dox/GiBUUTools.dox.cfg
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dox
  COMMENT "Generating documentation with Doxygen... (this will take a while)" VERBATIM
  )

  add_custom_target(docs
  make
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dox/latex
  COMMENT "Building latex documentation with Doxygen... (this will also take a while)" VERBATIM
  )
  add_dependencies(docs doc_generate)
  install(FILES ${CMAKE_BINARY_DIR}/dox/latex/refman.pdf
    DESTINATION ${CMAKE_BINARY_DIR}/dox
    RENAME GiBUUTools.pdf OPTIONAL)
endif(DOXYGEN_FOUND)
