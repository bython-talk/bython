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

# ---- Declare library ----
add_library(bython_lib OBJECT source/bython/lib.cpp)
add_subdirectory(source)

target_include_directories(
    bython_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(bython_lib PUBLIC cxx_std_17)
target_include_directories(bython_lib PRIVATE foonathan::lexy)
target_link_libraries(bython_lib PRIVATE foonathan::lexy::file)

target_include_directories(bython_lib SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})

llvm_map_components_to_libnames(LLVM_LIBS
  support core
  irreader mcjit executionengine interpreter
  native x86codegen)
target_link_libraries(bython_lib PRIVATE ${LLVM_LIBS})

target_compile_definitions(bython_lib PRIVATE ${LLVM_DEFINITIONS})
#target_link_options(bython_lib PRIVATE "LINKER:${LLVM_LDFLAGS}")



# ---- Declare executables ----
llvm_map_components_to_libnames(LLVM_SUPPORT_LIB support)
foreach(VARIATION IN ITEMS repl compiler jit)
  add_executable(bython_${VARIATION} source/${VARIATION}.cpp)
  add_executable(bython::${VARIATION} ALIAS bython_${VARIATION})

  target_compile_features(bython_${VARIATION} PRIVATE cxx_std_17)

  # Use command line parser given by LLVM
  target_include_directories(bython_${VARIATION} SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
  target_link_libraries(bython_${VARIATION} PRIVATE bython_lib ${LLVM_SUPPORT_LIB})

  set_property(TARGET bython_${VARIATION} PROPERTY OUTPUT_NAME bython-${VARIATION})
endforeach()

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
