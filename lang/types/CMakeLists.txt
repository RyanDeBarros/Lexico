set(TYPES_SOURCES
	datatype.cpp
	datapoint.cpp
	basic.cpp
	pattern.cpp
	declarations.cpp
	unresolved.cpp
)

target_sources(lexico-lang PRIVATE ${TYPES_SOURCES})
