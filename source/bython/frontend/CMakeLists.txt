add_library(bython_frontend)

target_sources(bython_frontend PRIVATE
    frontend.cpp
    lexy.cpp
)

target_include_directories(
    bython_frontend ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_include_directories(bython_frontend PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(bython_frontend PRIVATE bython_ast foonathan::lexy)
target_compile_features(bython_frontend PRIVATE cxx_std_20)