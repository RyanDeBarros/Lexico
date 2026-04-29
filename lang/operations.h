#pragma once

#include "token.h"
#include "symbols.h"

#include <optional>

namespace lx
{
	enum class DataType
	{
		Int,
		Float,
		Bool,
		String,
		Void,
		Pattern,
		Match,
		Matches,
		CapId,
		Cap,
		IRange,
		SRange,
		List,

		// Internal
		_Marker,
		_Scope,
		_Color,
	};

	extern DataType data_type(TokenType type);
	extern DataType data_type(BuiltinSymbol symbol);
	extern std::string friendly_name(DataType type);

	extern bool can_cast(DataType from, DataType to);
	extern bool is_iterable(DataType type);
	extern bool is_highlightable(DataType type);

	enum class StandardBinaryOperator
	{
		And,
		Asterisk,
		EqualTo,
		GreaterThan,
		GreaterThanOrEqualTo,
		LessThan,
		LessThanOrEqualTo,
		Minus,
		Mod,
		NotEqualTo,
		Or,
		Plus,
		Slash,
		To,
	};

	extern StandardBinaryOperator standard_binary_operator(TokenType type);

	extern std::optional<DataType> evaltype(StandardBinaryOperator op, DataType lhs, DataType rhs);

	enum class StandardPrefixOperator
	{
		Max,
		Min,
		Minus,
		Not,
	};

	extern StandardPrefixOperator standard_prefix_operator(TokenType type);

	extern std::optional<DataType> evaltype(StandardPrefixOperator op, DataType type);

	enum class PatternSimpleRepeatOperator
	{
		Asterisk,
		Plus,
	};

	extern PatternSimpleRepeatOperator pattern_simple_repeat_operator(TokenType type);

	enum class PatternPrefixOperator
	{
		Ahead,
		Behind,
		Max,
		Min,
		Not,
		NotAhead,
		NotBehind,
		Optional,
		Ref,
	};

	extern PatternPrefixOperator pattern_prefix_operator(TokenType type);

	enum class PatternBinaryOperator
	{
		Comma,
		Except,
		Or,
		Repeat,
		To,
	};

	extern PatternBinaryOperator pattern_binary_operator(TokenType type);
}
