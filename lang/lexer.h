#pragma once

#include "token.h"
#include "errors.h"

namespace lx
{
	class Lexer
	{
		TokenStream _stream;
		std::vector<std::string_view> _script_lines;
		std::vector<LxError> _errors;

	public:
		void tokenize(const std::string_view script);
		const TokenStream& stream() const;
		TokenStream& stream();
		const std::vector<std::string_view>& script_lines() const;
		const std::vector<LxError>& errors() const;
	};
}
