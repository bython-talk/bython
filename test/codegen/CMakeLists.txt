configure_file(lit.site.cfg.py.in lit.cfg.py @ONLY)

add_test(NAME bython_codegen_lit COMMAND lit ${CMAKE_BINARY_DIR}/test/codegen)

# Don't bother running if frontend tests fail
set_tests_properties(bython_codegen_lit PROPERTIES FIXTURES_REQUIRED bython_frontend)
