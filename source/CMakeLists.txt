target_sources(bython_lib PRIVATE
    lib.cpp
)
add_subdirectory(bython/ast)
# add_subdirectory(bython/codegen)
add_subdirectory(bython/matching)
add_subdirectory(bython/parser)
add_subdirectory(bython/visitation)
