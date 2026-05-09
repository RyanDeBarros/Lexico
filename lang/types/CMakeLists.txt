set(TYPES_SOURCES
	datatype.cpp
	datapoint.cpp
	declarations.cpp
	member.cpp
	iterator.cpp
	variable.cpp
	heap.cpp
	processing.cpp
	evalcontext.cpp
	symbolizer.cpp
)

target_sources(lexico-lang PRIVATE ${TYPES_SOURCES})

add_subdirectory(primitives)
