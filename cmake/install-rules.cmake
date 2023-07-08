install(
    TARGETS bython_exe
    RUNTIME COMPONENT bython_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
