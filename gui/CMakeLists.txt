include(imgui.cmake)

set(GUI_SOURCES
	main.cpp
)

add_executable(lexico-desktop ${GUI_SOURCES})
target_link_libraries(lexico-desktop PRIVATE lexico-lang imgui_backend)
