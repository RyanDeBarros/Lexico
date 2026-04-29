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

	class LxError : public std::runtime_error
	{
		ErrorType _type;
		std::string _message;

	public:
		LxError(ErrorType type, std::string&& message = "");

		ErrorType type() const;
		std::string message() const;

		static std::string underline(const ScriptSegment& segment, unsigned int tabs = 1);
		static LxError segment_error(const ScriptSegment& segment, ErrorType type, const std::string_view cause);
	};
}
