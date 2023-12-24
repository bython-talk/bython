install(TARGETS bython_driver RUNTIME COMPONENT bython_Runtime)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
