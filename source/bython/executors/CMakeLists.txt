add_library(bython_executors)

target_sources(bython_executors PRIVATE
    #interpreter.cpp
    jit.cpp
)


llvm_map_components_to_libnames(LLVM_EXECUTOR_LIBS
  core orcjit mcjit executionengine interpreter native)

target_include_directories(
    bython_executors ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_link_libraries(bython_executors PRIVATE bython_ast bython_frontend bython_backend ${LLVM_EXECUTOR_LIBS})
target_include_directories(bython_executors SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(bython_executors PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(bython_executors PRIVATE cxx_std_20)