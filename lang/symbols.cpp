#include "symbols.h"

#include "errors.h"

#include <sstream>

namespace lx
{
	std::optional<BuiltinSymbol> parse_builtin_symbol(const std::string_view symbol)
	{
		if (symbol == "%")
			return BuiltinSymbol::Percent;

		if (!symbol.starts_with('$'))
			return std::nullopt;

		const std::string_view name = symbol.substr(1);
		if (name == "alphanumeric")
			return BuiltinSymbol::Alphanumeric;
		else if (name == "any")
			return BuiltinSymbol::Any;
		else if (name == "digit")
			return BuiltinSymbol::Digit;
		else if (name == "end")
			return BuiltinSymbol::End;
		else if (name == "letter")
			return BuiltinSymbol::Letter;
		else if (name == "line")
			return BuiltinSymbol::Line;
		else if (name == "lines")
			return BuiltinSymbol::Lines;
		else if (name == "lowercase")
			return BuiltinSymbol::Lowercase;
		else if (name == "newline")
			return BuiltinSymbol::Newline;
		else if (name == "page")
			return BuiltinSymbol::Page;
		else if (name == "space")
			return BuiltinSymbol::Space;
		else if (name == "start")
			return BuiltinSymbol::Start;
		else if (name == "uppercase")
			return BuiltinSymbol::Uppercase;
		else if (name == "varname")
			return BuiltinSymbol::Varname;
		else if (name == "whitespace")
			return BuiltinSymbol::Whitespace;
		else if (name == "yellow")
			return BuiltinSymbol::Yellow;
		else if (name == "red")
			return BuiltinSymbol::Red;
		else if (name == "green")
			return BuiltinSymbol::Green;
		else if (name == "blue")
			return BuiltinSymbol::Blue;
		else if (name == "grey" || name == "gray")
			return BuiltinSymbol::Grey;
		else if (name == "purple")
			return BuiltinSymbol::Purple;
		else if (name == "orange")
			return BuiltinSymbol::Orange;
		else if (name == "mono")
			return BuiltinSymbol::Mono;
		else
			return std::nullopt;
	}

	MarkerIdentifier marker(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
		case BuiltinSymbol::Any:
			return MarkerIdentifier::Any;
		case BuiltinSymbol::End:
			return MarkerIdentifier::End;
		case BuiltinSymbol::Start:
			return MarkerIdentifier::Start;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": unrecognized marker " << static_cast<int>(symbol);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	Marker::Marker(MarkerIdentifier identifier)
		: _identifier(identifier)
	{
	}

	Scope::Scope(std::optional<unsigned int> lines)
		: _lines(lines)
	{
	}

	std::optional<unsigned int> Scope::lines() const
	{
		return _lines;
	}

	bool is_scope_symbol(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
		case BuiltinSymbol::Line:
		case BuiltinSymbol::Lines:
		case BuiltinSymbol::Page:
			return true;
		default:
			return false;
		}
	}

	static HighlightColor convert_color(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
		case BuiltinSymbol::Yellow:
			return HighlightColor::Yellow;
		case BuiltinSymbol::Red:
			return HighlightColor::Red;
		case BuiltinSymbol::Green:
			return HighlightColor::Green;
		case BuiltinSymbol::Blue:
			return HighlightColor::Blue;
		case BuiltinSymbol::Grey:
			return HighlightColor::Grey;
		case BuiltinSymbol::Purple:
			return HighlightColor::Purple;
		case BuiltinSymbol::Orange:
			return HighlightColor::Orange;
		case BuiltinSymbol::Mono:
			return HighlightColor::Mono;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": could not convert symbol " << static_cast<int>(symbol) << " to highlight color";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	Color::Color(BuiltinSymbol symbol)
		: _color(convert_color(symbol))
	{
	}

	HighlightColor Color::color() const
	{
		return _color;
	}

	bool is_color_symbol(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
		case BuiltinSymbol::Yellow:
		case BuiltinSymbol::Red:
		case BuiltinSymbol::Green:
		case BuiltinSymbol::Blue:
		case BuiltinSymbol::Grey:
		case BuiltinSymbol::Purple:
		case BuiltinSymbol::Orange:
		case BuiltinSymbol::Mono:
			return true;
		default:
			return false;
		}
	}
}
