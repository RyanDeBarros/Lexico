set(TYPES_SOURCES
	datatype.cpp
	datapoint.cpp
	basic.cpp
	pattern.cpp
	declarations.cpp
	member.cpp
	iterator.cpp
	variable.cpp
	datapath.cpp
	heap.cpp
	processing.cpp
)

target_sources(lexico-lang PRIVATE ${TYPES_SOURCES})
