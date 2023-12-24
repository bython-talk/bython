add_library(bython_test_stringmaker)

target_sources(bython_test_stringmaker PRIVATE
    type.cpp)

target_link_libraries(bython_test_stringmaker PRIVATE bython_type_system Catch2::Catch2)
target_compile_features(bython_test_stringmaker PRIVATE cxx_std_20)