set(LANG_SOURCES
	lang.cpp
	lexer.cpp
	parser.cpp
	semantics.cpp
	executor.cpp
	resolution.cpp
	runtime.cpp
	errors.cpp
	token.cpp
	ast.cpp
	operations.cpp
	symbols.cpp
	util.cpp
)

add_library(lexico-lang ${LANG_SOURCES} "lang.h")

target_include_directories(lexico-lang PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(lexico-lang PUBLIC cxx_std_20)

add_subdirectory(types)
