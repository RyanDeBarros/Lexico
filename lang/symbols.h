#pragma once

#include <string_view>
#include <optional>

namespace lx
{
	enum class BuiltinSymbol
	{
		Percent,
		Alphanumeric,
		Any,
		Cap,
		Digit,
		End,
		Letter,
		Line,
		Lines,
		Lowercase,
		Newline,
		Page,
		Space,
		Start,
		Uppercase,
		Varname,

		// Colors
		Yellow,
		Red,
		Green,
		Blue,
		Grey,
		Purple,
		Orange,
		Mono,
	};

	extern std::optional<BuiltinSymbol> parse_builtin_symbol(const std::string_view symbol);
}
