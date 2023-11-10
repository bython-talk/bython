cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    bython
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES C CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# --- Third Party Deps ---
find_package(lexy REQUIRED)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS})
message(STATUS LLVM_LIBRARY_DIRS: ${LLVM_LIBRARY_DIRS})
message(STATUS LLVM_DEFINITIONS: ${LLVM_DEFINITIONS})
# message(STATUS LLVM_AVAILABLE_LIBS: ${LLVM_AVAILABLE_LIBS})


find_package(Boost REQUIRED)
message(STATUS Boost: ${Boost_INCLUDE_DIRS})
add_subdirectory(source)

# ---- Declare executables ----
llvm_map_components_to_libnames(LLVM_SUPPORT_LIB support core)
add_executable(bython_driver source/main.cpp)
add_executable(bython::driver ALIAS bython_driver)

target_compile_features(bython_driver PRIVATE cxx_std_20)

# Use command line parser given by LLVM
target_include_directories(bython_driver SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(bython_driver PRIVATE bython_executors ${LLVM_SUPPORT_LIB})

set_property(TARGET bython_driver PROPERTY OUTPUT_NAME bython-driver)

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
