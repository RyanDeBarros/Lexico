#pragma once

#include "token.h"
#include "symbols.h"
#include "types/datatype.h"

namespace lx
{
	extern DataType data_type(Keyword simple_type, const std::vector<Keyword>& underlying_types);
	extern DataType literal_type(TokenType type);

	enum class BinaryOperator
	{
		And,
		Assign,
		Asterisk,
		Comma,
		EqualTo,
		Except,
		GreaterThan,
		GreaterThanOrEqualTo,
		LessThan,
		LessThanOrEqualTo,
		Minus,
		Mod,
		NotEqualTo,
		Or,
		Plus,
		Repeat,
		Slash,
		To,
	};

	extern BinaryOperator binary_operator(const Token& token);
	extern std::optional<DataType> evaltype(BinaryOperator op, const DataType& lhs, const DataType& rhs);
	extern bool is_imperative(BinaryOperator op);

	enum class PrefixOperator
	{
		Ahead,
		Behind,
		Max,
		Min,
		Minus,
		Not,
		NotAhead,
		NotBehind,
		Optional,
		Ref,
	};

	extern PrefixOperator prefix_operator(const Token& token);
	extern std::optional<DataType> evaltype(PrefixOperator op, const DataType& type);

	enum class PatternSimpleRepeatOperator
	{
		Asterisk,
		Plus,
	};

	extern PatternSimpleRepeatOperator pattern_simple_repeat_operator(TokenType type);
}
