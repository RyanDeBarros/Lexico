set(TYPES_SOURCES
	datatype.cpp
	datapoint.cpp
	basic.cpp
	pattern.cpp
	declarations.cpp
	member.cpp
	variable.cpp
	heap.cpp
	accessor.cpp
	processing.cpp
)

target_sources(lexico-lang PRIVATE ${TYPES_SOURCES})
