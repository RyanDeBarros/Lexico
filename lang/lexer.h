#pragma once

#include "token.h"

namespace lx
{
	class Lexer
	{
		TokenStream _stream;
		std::vector<std::string_view> _script_lines;

	public:
		void tokenize(const std::string_view script);
		const TokenStream& stream() const;
		TokenStream& stream();
		const std::vector<std::string_view>& script_lines() const;
	};
}
