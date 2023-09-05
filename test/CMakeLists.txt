# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(bythonTests LANGUAGES CXX)


find_package(Catch2 3 REQUIRED)

# ---- Catch2 Tests ----

add_executable(bython_scratchpad_test frontend/lexy/string_frontend.cpp)
target_link_libraries(bython_scratchpad_test PRIVATE
        bython_lib
        Catch2::Catch2WithMain)
target_compile_features(bython_scratchpad_test PRIVATE cxx_std_20)


add_executable(bython_test)
target_link_libraries(bython_test PRIVATE
        bython_lib
        Catch2::Catch2WithMain)
target_compile_features(bython_test PRIVATE cxx_std_20)

include(CTest)
include(Catch)
catch_discover_tests(bython_test)

add_test(NAME bython_test COMMAND bython_test)
set_tests_properties(bython_test PROPERTIES FIXTURES_SETUP bython_frontend)

add_subdirectory(lexy_frontend)
add_subdirectory(llvm_backend)
add_subdirectory(matching)
add_subdirectory(visitation)

# ---- End-of-file commands ----

add_folders(Test)
