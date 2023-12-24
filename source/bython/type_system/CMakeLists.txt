add_library(bython_type_system)

target_sources(bython_type_system PRIVATE 
    builtin.cpp
    environment.cpp 
    inference.cpp
    subtyping.cpp
)

target_link_libraries(bython_type_system PRIVATE bython_ast)
target_include_directories(
    bython_type_system ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_link_libraries(bython_type_system PRIVATE bython_ast)
target_compile_features(bython_type_system PRIVATE cxx_std_20)