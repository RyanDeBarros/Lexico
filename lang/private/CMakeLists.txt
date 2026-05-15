set(ROOT_SOURCES
	lexico.cpp
	lexer.cpp
	parser.cpp
	semantics.cpp
	executor.cpp
	resolution.cpp
	ftable.cpp
	runtime.cpp
	errors.cpp
	token.cpp
	ast.cpp
	operations.cpp
	find.cpp
	symbols.cpp
	page.cpp
	highlights.cpp
	util.cpp
)

target_sources(lexico-lang PRIVATE ${ROOT_SOURCES})

add_subdirectory(types)
