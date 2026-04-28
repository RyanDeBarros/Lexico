#pragma once

#include "token.h"

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
	};

	extern DataType data_type(TokenType type);
	extern std::string friendly_name(DataType type);

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
