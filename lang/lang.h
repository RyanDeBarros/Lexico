#pragma once

#include <string>
#include <string_view>

namespace lx
{
	// TODO v0.2 move other files in internal subfolders, and add new 'api' subfolder for use of lexico in code

	extern bool execute(const std::string_view script, const std::string_view input, std::string& output, std::string& log);
}
