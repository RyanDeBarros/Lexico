set(LANG_SOURCES
	lang.cpp
	token.cpp
	lexer.cpp
	parser.cpp
)

add_library(lexico-lang ${LANG_SOURCES} "lang.h")

target_include_directories(lexico-lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(lexico-lang PUBLIC cxx_std_20)
