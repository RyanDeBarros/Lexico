#pragma once

#include "highlights.h"

#include <optional>
#include <string_view>

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
		Whitespace,

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

	enum class MarkerIdentifier
	{
		Any,
		Cap,
		End,
		Start,
	};

	extern MarkerIdentifier marker(BuiltinSymbol symbol);

	class Marker
	{
		MarkerIdentifier _identifier;

	public:
		explicit Marker(MarkerIdentifier identifier);
	};

	class Scope
	{
		std::optional<unsigned int> _lines;

	public:
		explicit Scope(std::optional<unsigned int> lines);

		std::optional<unsigned int> lines() const;
	};

	extern bool is_scope_symbol(BuiltinSymbol symbol);

	class Color
	{
		HighlightColor _color;

	public:
		explicit Color(BuiltinSymbol symbol);

		HighlightColor color() const;
	};

	extern bool is_color_symbol(BuiltinSymbol symbol);
}
