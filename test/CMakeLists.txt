# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(bythonTests LANGUAGES CXX)


find_package(Catch2 3 REQUIRED)

# ---- Tests ----

add_executable(bython_test)
add_subdirectory(parser)

target_link_libraries(bython_test PRIVATE
        bython_lib
        Catch2::Catch2WithMain)
target_compile_features(bython_test PRIVATE cxx_std_17)

include(CTest)
include(Catch)
catch_discover_tests(bython_test)

add_test(NAME bython_test COMMAND bython_test)

# ---- End-of-file commands ----

add_folders(Test)
