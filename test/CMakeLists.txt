# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(bythonTests LANGUAGES CXX)


find_package(Catch2 3 REQUIRED)

# ---- Catch2 Tests ----

add_executable(bython_test)

add_subdirectory(matching)
add_subdirectory(parser)
add_subdirectory(visitation)

target_link_libraries(bython_test PRIVATE
        bython_lib
        Catch2::Catch2WithMain)
target_compile_features(bython_test PRIVATE cxx_std_17)

include(CTest)
include(Catch)
catch_discover_tests(bython_test)

add_test(NAME bython_test COMMAND bython_test)
set_tests_properties(bython_test   PROPERTIES FIXTURES_SETUP    bython_frontend)

# ---- LIT Tests ----
add_test(NAME bython_codegen_lit COMMAND lit ${CMAKE_SOURCE_DIR}/test/codegen)

# Don't bother running if frontend tests fail
set_tests_properties(bython_codegen_lit PROPERTIES FIXTURES_REQUIRED bython_frontend)



# ---- End-of-file commands ----

add_folders(Test)
