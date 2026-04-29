#pragma once

#include <string_view>
#include <optional>

namespace lx
{
	enum class BuiltinSymbol
	{
		// Pattern
		Alphanumeric,
		Digit,
		Letter,
		Lowercase,
		Newline,
		Space,
		Uppercase,
		Varname,

		// Marker
		Any,
		Cap,
		End,
		Start,

		// Matches
		Percent,

		// Scope
		Line,
		Lines,
		Page,

		// Color
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
