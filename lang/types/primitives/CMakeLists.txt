set(TYPES_PRIMITIVES_SOURCES
	int.cpp
	float.cpp
	bool.cpp
	string.cpp
	void.cpp
	pattern.cpp
	match.cpp
	matches.cpp
	capid.cpp
	cap.cpp
	irange.cpp
	srange.cpp
	list.cpp
)

target_sources(lexico-lang PRIVATE ${TYPES_PRIMITIVES_SOURCES})
