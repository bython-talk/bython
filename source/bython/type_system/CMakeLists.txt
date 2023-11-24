add_library(bython_type_system)

target_sources(bython_type_system PRIVATE 
    environment.cpp 
    inference.cpp
    subtyping.cpp
)

llvm_map_components_to_libnames(LLVM_BACKEND_LIBS
  support core irreader)

target_link_libraries(bython_type_system PRIVATE bython_ast)
target_include_directories(
    bython_type_system ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_link_libraries(bython_backend PRIVATE bython_ast ${LLVM_BACKEND_LIBS})
target_include_directories(bython_type_system SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(bython_type_system PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(bython_type_system PRIVATE cxx_std_20)