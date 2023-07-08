# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(bythonTests LANGUAGES CXX)


Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.3.2 # or a later release
)

FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

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
