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
		Token start_token() const;
		static std::vector<std::string_view> split_lines(const std::string_view sv);
		const std::vector<std::string_view>& script_lines() const;
		std::vector<std::string_view>& script_lines();
		const std::vector<LxError>& errors() const;
	};
}
