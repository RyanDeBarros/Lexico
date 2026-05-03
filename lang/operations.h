#pragma once

#include "token.h"
#include "symbols.h"

#include <optional>
#include <variant>

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
	extern DataType literal_type(TokenType type);
	extern DataType data_type(BuiltinSymbol symbol);
	extern std::string friendly_name(DataType type);

	extern std::ostream& operator<<(std::ostream& os, DataType type);

	struct MemberSignature
	{
		struct DataLayout
		{
			DataType type;
		};

		struct Overload
		{
			DataType return_type;
			std::vector<DataType> arg_types;
		};

		struct MethodLayout
		{
			std::vector<Overload> overloads;
		};

		std::string identifier;
		std::variant<DataLayout, MethodLayout> layout;

		static MemberSignature make_data(std::string&& identifier, DataType type);
		static MemberSignature make_method(std::string&& identifier, std::vector<Overload>&& overloads);

		bool is_data() const;
		bool is_method() const;

		DataType data_type() const;
		const std::vector<Overload>& method_overloads() const;
		std::optional<DataType> return_type(const std::vector<DataType>& arg_types) const;
	};

	extern const std::vector<MemberSignature>* data_type_members(DataType type);

	extern bool can_cast_implicit(DataType from, DataType to);
	extern bool can_cast_explicit(DataType from, DataType to);
	extern bool is_iterable(DataType type);
	extern bool is_highlightable(DataType type);
	extern bool is_pageable(DataType type);

	enum class BinaryOperator
	{
		And,
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

	extern std::optional<DataType> evaltype(BinaryOperator op, DataType lhs, DataType rhs);

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

	extern std::optional<DataType> evaltype(PrefixOperator op, DataType type);

	enum class PatternSimpleRepeatOperator
	{
		Asterisk,
		Plus,
	};

	extern PatternSimpleRepeatOperator pattern_simple_repeat_operator(TokenType type);
}
