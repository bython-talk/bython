add_library(bython_backend)

target_sources(bython_backend PRIVATE
    builtin.cpp
    intrinsic.cpp
    llvm.cpp
    symbols.cpp
    typing.cpp
)


llvm_map_components_to_libnames(LLVM_BACKEND_LIBS
  support core irreader native x86codegen)

target_include_directories(
    bython_backend ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_link_libraries(bython_backend PRIVATE bython_ast bython_type_system ${LLVM_BACKEND_LIBS})
target_include_directories(bython_backend SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(bython_backend PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(bython_backend PRIVATE cxx_std_20)