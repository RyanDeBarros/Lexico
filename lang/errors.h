#pragma once

#include <stdexcept>

#include "token.h"

namespace lx
{
	enum class ErrorType
	{
		Syntax,
		Semantic,
		Internal,
	};

	struct LxError : std::runtime_error
	{
		LxError(ErrorType type, std::string&& message = "");

		static std::string underline(const Token& token, unsigned int tabs = 1);
		static LxError token_error(const Token& token, const std::vector<std::string_view>& script_lines, ErrorType type, const std::string_view cause);
	};
}
