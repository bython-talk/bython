add_library(bython_ast)

target_sources(
    bython_ast PRIVATE 
    bases.cpp
    expression.cpp 
    module.cpp 
    operators.cpp 
    statement.cpp 
    tags.cpp)

target_include_directories(
    bython_ast ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)
target_include_directories(bython_ast PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_features(bython_ast PRIVATE cxx_std_20)