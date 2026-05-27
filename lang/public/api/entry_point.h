#pragma once

#include <string>
#include <string_view>

#include "highlights.h"

namespace lx
{
	struct ExecInput
	{
		std::string_view script;
		std::string_view input;
		// TODO config variables
	};

	struct ExecResult
	{
		bool success = false;
		std::string output;
		std::string log;
		HighlightMap highlights;
	};
}
