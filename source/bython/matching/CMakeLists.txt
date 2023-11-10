add_library(bython_matching)

target_sources(bython_matching PRIVATE 
    bases.cpp 
    expression.cpp 
    operators.cpp 
    special.cpp 
    statement.cpp)

target_link_libraries(bython_matching PRIVATE bython_ast)
target_include_directories(
    bython_matching ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)
target_compile_features(bython_matching PRIVATE cxx_std_20)