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

		static std::string underline(const ScriptSegment& segment, unsigned int tabs = 1);
		static LxError segment_error(const ScriptSegment& segment, ErrorType type, const std::string_view cause);
	};
}
