set(TYPES_SOURCES
	datatype.cpp
	datapoint.cpp
	basic.cpp
	pattern.cpp
	declarations.cpp
	member.cpp
	iterator.cpp
	variable.cpp
	heap.cpp
	datapath.cpp
	processing.cpp
	evalcontext.cpp
)

target_sources(lexico-lang PRIVATE ${TYPES_SOURCES})
