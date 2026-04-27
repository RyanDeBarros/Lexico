#include "symbols.h"

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
		else if (name == "cap")
			return BuiltinSymbol::Cap;
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
}
