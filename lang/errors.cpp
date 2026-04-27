#include "errors.h"

#include <sstream>

namespace lx
{
	static std::string error_prefix(ErrorType type)
	{
		switch (type)
		{
		case lx::ErrorType::Syntax:
			return "[Syntax Error] ";
		case lx::ErrorType::Semantic:
			return "[Semantic Error] ";
		case lx::ErrorType::Internal:
			return "[Internal Error] ";
		default:
			return "";
		}
	}

	LxError::LxError(ErrorType type, std::string&& message)
		: std::runtime_error(error_prefix(type) + std::move(message))
	{
	}

	std::string LxError::underline(const Token& token, unsigned int tabs)
	{
		std::stringstream ss;

		while (tabs > 0)
		{
			ss << "    ";
			--tabs;
		}
		for (unsigned int i = 1; i < token.start_column; ++i)
			ss << ' ';
		ss << '^';
		if (token.start_line == token.end_line)
		{
			for (unsigned int i = token.start_column + 1; i <= token.end_column; ++i)
				ss << '~';
		}
		return ss.str();
	}

	LxError LxError::token_error(const Token& token, const std::vector<std::string_view>& script_lines, ErrorType type, const std::string_view cause)
	{
		std::stringstream ss;
		ss << cause;
		ss << ":\n";
		std::string line_number = token.line_number_prefix();
		unsigned int tabs = 1 + line_number.size() / 4;
		ss << "    " << line_number << script_lines[token.start_line - 1] << '\n' << LxError::underline(token, tabs);
		return LxError(type, ss.str());
	}
}
