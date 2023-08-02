install(
    TARGETS bython_repl bython_compiler bython_jit
    RUNTIME COMPONENT bython_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
