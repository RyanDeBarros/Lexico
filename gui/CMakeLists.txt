include(imgui.cmake)

set(GUI_SOURCES
	main.cpp
	state.cpp
	actions.cpp
)

add_executable(lexico-desktop ${GUI_SOURCES})
target_link_libraries(lexico-desktop PRIVATE lexico-lang imgui_backend)
