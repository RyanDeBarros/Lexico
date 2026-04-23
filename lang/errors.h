#pragma once

#include <stdexcept>

#include "token.h"

namespace lx
{
	struct SyntaxError : std::runtime_error
	{
		SyntaxError(std::string&& message);

		static std::string underline(const Token& token, unsigned int tabs = 1);
	};
}
