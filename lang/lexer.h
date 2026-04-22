#pragma once

#include "token.h"

namespace lx
{
	class Lexer
	{
		TokenStream _stream;

	public:
		void tokenize(const std::string_view script);
		const TokenStream& stream() const;
		TokenStream& stream();
	};
}
