#pragma once

#include <stdexcept>

#include "token.h"

namespace lx
{
	enum class ErrorType
	{
		Syntax,
		Semantic,
	};

	struct LxError : std::runtime_error
	{
		LxError(ErrorType type, std::string&& message);

		static std::string underline(const Token& token, unsigned int tabs = 1);
	};
}
