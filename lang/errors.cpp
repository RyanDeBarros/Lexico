#include "errors.h"

#include <sstream>

namespace lx
{
	SyntaxError::SyntaxError(std::string&& message)
		: std::runtime_error(std::move(message))
	{
	}

	std::string SyntaxError::underline(const Token& token, unsigned int tabs)
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
}
