#pragma once

#include <string>
#include <string_view>

namespace lx
{
	// TODO v0.2 move other files in internal subfolders, and add new 'api' subfolder for use of lexico in code

	struct ExecInput
	{
		std::string_view script;
		std::string_view input;
		// TODO v0.2 config variables
	};

	struct ExecResult
	{
		bool success;
		std::string output;
		std::string log;
		// TODO highlights
	};

	extern ExecResult execute(const ExecInput& input);
}
