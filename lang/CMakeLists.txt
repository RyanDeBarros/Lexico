set(LANG_HEADERS
	lang.h
)

set(LANG_SOURCES
	lang.cpp
)

add_library(lexico-lang ${LANG_SOURCES} ${LANG_HEADERS})

target_include_directories(lexico-lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(lexico-lang PUBLIC cxx_std_20)
