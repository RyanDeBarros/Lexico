add_library(lexico-lang STATIC)

add_subdirectory(public)
add_subdirectory(private)

target_compile_features(lexico-lang PUBLIC cxx_std_20)
target_include_directories(lexico-lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(lexico-lang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)
