include(cmake/folders.cmake)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

add_custom_target(run-repl COMMAND bython-repl VERBATIM)
add_dependencies(run-repl bython_repl)

add_custom_target(run-compiler COMMAND bython-compiler VERBATIM)
add_dependencies(run-compiler bython_compiler)

add_custom_target(run-jit COMMAND bython-jit VERBATIM)
add_dependencies(run-jit bython_jit)

option(BUILD_MCSS_DOCS "Build documentation using Doxygen and m.css" OFF)
if(BUILD_MCSS_DOCS)
  include(cmake/docs.cmake)
endif()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if(ENABLE_COVERAGE)
  include(cmake/coverage.cmake)
endif()

include(cmake/lint-targets.cmake)
include(cmake/spell-targets.cmake)

add_folders(Project)
