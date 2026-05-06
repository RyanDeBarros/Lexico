#pragma once

#include <string>
#include <string_view>

namespace lx
{
	struct ExecInput
	{
		std::string_view script;
		std::string_view input;
		// TODO v0.2 config variables
	};

	struct ExecResult
	{
		bool success = false;
		std::string output;
		std::string log;
		// TODO highlights
	};

	enum class HighlightColor
	{
		Yellow,
		Red,
		Green,
		Blue,
		Grey,
		Purple,
		Orange,
		Mono,
	};
}
