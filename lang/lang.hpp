#pragma once

#include <string>
#include <string_view>

namespace lx
{
	extern bool execute(const std::string_view input, std::string& output, std::string& log);
}
