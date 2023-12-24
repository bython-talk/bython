# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(bythonTests LANGUAGES CXX)


find_package(Catch2 3 REQUIRED)

# ---- Catch2 Tests ----

#add_executable(bython_scratchpad_test frontend/lexy/string_frontend.cpp)
#target_link_libraries(bython_scratchpad_test PRIVATE
#        bython_lib
#        Catch2::Catch2WithMain)
#target_compile_features(bython_scratchpad_test PRIVATE cxx_std_20)

add_subdirectory(utils)

add_executable(bython_test_type_system type_system/inference.cpp type_system/subtyping.cpp)
target_link_libraries(bython_test_type_system PRIVATE
        bython_type_system bython_frontend bython_ast
        bython_test_stringmaker
        Catch2::Catch2WithMain)
target_compile_features(bython_test_type_system PRIVATE cxx_std_20)
target_include_directories(bython_test_type_system PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/utils")

include(Catch)
catch_discover_tests(bython_test_type_system)

add_test(NAME bython_test_type_system COMMAND bython_test_type_system)

#add_test(NAME bython_test COMMAND bython_test)
#set_tests_properties(bython_test PROPERTIES FIXTURES_SETUP bython_frontend)


# -- Lit Testing
configure_file(lit.site.cfg.py.in lit.cfg.py @ONLY)

foreach(SNAPSHOT_TESTING IN ITEMS backend)
    add_test(
        NAME bython_test_${SNAPSHOT_TESTING} 
        COMMAND lit -v ${CMAKE_BINARY_DIR}/test/${SNAPSHOT_TESTING}
    )
    
endforeach()

#add_test(NAME bython_frontend_lit COMMAND lit -v ${CMAKE_BINARY_DIR}/test/frontend)

# Don't bother running if frontend tests fail
#set_tests_properties(bython_test_typing PROPERTIES FIXTURES_REQUIRED bython_test_frontend)
set_tests_properties(bython_test_backend PROPERTIES FIXTURES_REQUIRED bython_test_type_system)



# ---- End-of-file commands ----

add_folders(Test)
