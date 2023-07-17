cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    bython
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES  C CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# --- Third Party Deps ---
include(FetchContent)
FetchContent_Declare(lexy URL https://github.com/foonathan/lexy/releases/download/v2022.12.1/lexy-src.zip)
FetchContent_MakeAvailable(lexy)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader)
# ---- Declare library ----

add_library(
    bython_lib OBJECT
    source/lib.cpp
)


target_include_directories(
    bython_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(bython_lib PUBLIC cxx_std_17)
target_link_libraries(bython_lib PUBLIC foonathan::lexy ${llvm_libs})


# ---- Declare executable ----

add_executable(bython_exe source/main.cpp)
add_executable(bython::exe ALIAS bython_exe)
add_subdirectory(source)

set_property(TARGET bython_exe PROPERTY OUTPUT_NAME bython)

target_compile_features(bython_exe PRIVATE cxx_std_17)
target_link_libraries(bython_exe PRIVATE bython_lib ${llvm_libs})


# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT bython_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of bython"
  )
endif()

include(cmake/dev-mode.cmake)
