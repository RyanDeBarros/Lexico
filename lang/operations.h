#pragma once

#include "token.h"
#include "symbols.h"
#include "types/datatype.h"

namespace lx
{
	extern DataType data_type(TokenType simple_type, const std::vector<TokenType>& underlying_types);
	extern DataType literal_type(TokenType type);

	extern bool can_cast_implicit(const DataType& from, const DataType& to);
	extern bool can_cast_explicit(const DataType& from, const DataType& to);
	extern bool is_iterable(const DataType& type);
	extern bool is_highlightable(const DataType& type);
	extern bool is_pageable(const DataType& type);

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

	extern BinaryOperator binary_operator(TokenType type);
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

	extern PrefixOperator prefix_operator(TokenType type);
	extern std::optional<DataType> evaltype(PrefixOperator op, const DataType& type);

	enum class PatternSimpleRepeatOperator
	{
		Asterisk,
		Plus,
	};

	extern PatternSimpleRepeatOperator pattern_simple_repeat_operator(TokenType type);
}
