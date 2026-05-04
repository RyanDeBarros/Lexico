#include "operations.h"

#include "errors.h"

#include <sstream>

namespace lx
{
	DataType data_type(TokenType type)
	{
		switch (type)
		{
		case TokenType::IntType:
			return DataType::Int;
		case TokenType::FloatType:
			return DataType::Float;
		case TokenType::BoolType:
			return DataType::Bool;
		case TokenType::StringType:
			return DataType::String;
		case TokenType::VoidType:
			return DataType::Void;
		case TokenType::PatternType:
			return DataType::Pattern;
		case TokenType::MatchType:
			return DataType::Match;
		case TokenType::MatchesType:
			return DataType::Matches;
		case TokenType::CapIdType:
			return DataType::CapId;
		case TokenType::CapType:
			return DataType::Cap;
		case TokenType::IRangeType:
			return DataType::IRange;
		case TokenType::SRangeType:
			return DataType::SRange;
		case TokenType::ListType:
			return DataType::List;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	DataType literal_type(TokenType type)
	{
		switch (type)
		{
		case TokenType::Integer:
			return DataType::Int;
		case TokenType::Float:
			return DataType::Float;
		case TokenType::String:
			return DataType::String;
		case TokenType::Bool:
			return DataType::Bool;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": literal type " << static_cast<int>(type) << " is not convertible to data type";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	DataType data_type(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
		case BuiltinSymbol::Alphanumeric:
		case BuiltinSymbol::Digit:
		case BuiltinSymbol::Letter:
		case BuiltinSymbol::Lowercase:
		case BuiltinSymbol::Newline:
		case BuiltinSymbol::Space:
		case BuiltinSymbol::Uppercase:
		case BuiltinSymbol::Varname:
			return DataType::Pattern;

		case BuiltinSymbol::Any:
		case BuiltinSymbol::Cap:
		case BuiltinSymbol::End:
		case BuiltinSymbol::Start:
			return DataType::_Marker;

		case BuiltinSymbol::Percent:
			return DataType::Matches;

		case BuiltinSymbol::Line:
		case BuiltinSymbol::Lines:
		case BuiltinSymbol::Page:
			return DataType::_Scope;

		case BuiltinSymbol::Yellow:
		case BuiltinSymbol::Red:
		case BuiltinSymbol::Green:
		case BuiltinSymbol::Blue:
		case BuiltinSymbol::Grey:
		case BuiltinSymbol::Purple:
		case BuiltinSymbol::Orange:
		case BuiltinSymbol::Mono:
			return DataType::_Color;

		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": unrecognized symbol " << static_cast<int>(symbol);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	MemberSignature MemberSignature::make_data(std::string&& identifier, DataType type)
	{
		return { .identifier = std::move(identifier), .layout = DataLayout{.type = type } };
	}

	MemberSignature MemberSignature::make_method(std::string&& identifier, std::vector<Overload>&& overloads)
	{
		return { .identifier = std::move(identifier), .layout = MethodLayout{.overloads = std::move(overloads) } };
	}

	bool MemberSignature::is_data() const
	{
		return std::holds_alternative<DataLayout>(layout);
	}

	bool MemberSignature::is_method() const
	{
		return std::holds_alternative<MethodLayout>(layout);
	}

	DataType MemberSignature::data_type() const
	{
		return std::get<DataLayout>(layout).type;
	}

	const std::vector<MemberSignature::Overload>& MemberSignature::method_overloads() const
	{
		return std::get<MethodLayout>(layout).overloads;
	}

	std::optional<DataType> MemberSignature::return_type(const std::vector<DataType>& arg_types) const
	{
		const auto& overloads = method_overloads();
		for (const auto& overload : overloads)
			if (overload.arg_types == arg_types)
				return overload.return_type;

		return std::nullopt;
	}

	static const std::unordered_map<DataType, StringMap<MemberSignature>> MEMBER_TABLE = {
		{ DataType::String, {
			{ "len", MemberSignature::make_data("len", DataType::Int) },
			{ "[]", MemberSignature::make_method("[]", {
				{.return_type = DataType::String, .arg_types = { DataType::Int } },
				{.return_type = DataType::String, .arg_types = { DataType::IRange } },
			}) },
		} },
		{ DataType::Match, {
			{ "caps", MemberSignature::make_data("caps", DataType::List) },
			{ "start", MemberSignature::make_data("start", DataType::Int) },
			{ "end", MemberSignature::make_data("end", DataType::Int) },
			{ "len", MemberSignature::make_data("len", DataType::Int) },
			{ "pos", MemberSignature::make_data("pos", DataType::IRange) },
			{ "str", MemberSignature::make_data("str", DataType::String) },
			{ "[]", MemberSignature::make_method("[]", {
				{ .return_type = DataType::Cap, .arg_types = { DataType::CapId } },
				{ .return_type = DataType::Cap, .arg_types = { DataType::Int } },
			}) },
		} },
		{ DataType::Cap, {
			{ "exists", MemberSignature::make_data("exists", DataType::Bool) },
			{ "start", MemberSignature::make_data("start", DataType::Int) },
			{ "end", MemberSignature::make_data("end", DataType::Int) },
			{ "len", MemberSignature::make_data("len", DataType::Int) },
			{ "pos", MemberSignature::make_data("pos", DataType::IRange) },
			{ "str", MemberSignature::make_data("str", DataType::String) },
			{ "sub", MemberSignature::make_data("sub", DataType::Match) },
		} },
		{ DataType::List, {
			{ "len", MemberSignature::make_data("len", DataType::Int)},
			{ "[]", MemberSignature::make_method("[]", {
				{ .return_type = DataType::_Unresolved, .arg_types = { DataType::Int } },
			}) },
		} },
	};

	const StringMap<MemberSignature>* data_type_members(DataType type)
	{
		auto it = MEMBER_TABLE.find(type);
		if (it != MEMBER_TABLE.end())
			return &it->second;
		else
			return nullptr;
	}

	bool can_cast_implicit(DataType from, DataType to)
	{
		if (to == DataType::Void || from == to || from == DataType::_Unresolved)
			return true;

		switch (from)
		{
		case DataType::Int:
			return to == DataType::Float || to == DataType::Bool || to == DataType::IRange;

		case DataType::Float:
			return to == DataType::Int || to == DataType::Bool;

		case DataType::Bool:
			return to == DataType::Int || to == DataType::Float;

		case DataType::String:
			return to == DataType::Pattern;

		case DataType::Pattern:
		case DataType::Void:
		case DataType::Match:
		case DataType::Matches:
		case DataType::CapId:
		case DataType::Cap:
		case DataType::IRange:
			return false;

		case DataType::SRange:
			return to == DataType::Pattern;

		case DataType::List:
			return false;

		case DataType::_Marker:
			return to == DataType::Pattern;

		case DataType::_Scope:
		case DataType::_Color:
		default:
			return false;
		}
	}

	bool can_cast_explicit(DataType from, DataType to)
	{
		if (can_cast_implicit(from, to))
			return true;

		switch (from)
		{
		case DataType::Int:
		case DataType::Float:
			return to == DataType::String || to == DataType::Pattern;

		case DataType::Bool:
			return to == DataType::String;

		case DataType::String:
			return to == DataType::Int || to == DataType::Float || to == DataType::Bool;

		case DataType::Pattern:
		case DataType::Void:
		case DataType::Match:
		case DataType::Matches:
		case DataType::CapId:
		case DataType::Cap:
		case DataType::IRange:
			return false;

		case DataType::SRange:
			return to == DataType::String;

		case DataType::List:
		case DataType::_Marker:
		case DataType::_Scope:
		case DataType::_Color:
		default:
			return false;
		}
	}

	bool is_iterable(DataType type)
	{
		switch (type)
		{
		case DataType::String:
		case DataType::Matches:
		case DataType::Match:
		case DataType::IRange:
		case DataType::SRange:
		case DataType::List:
		case DataType::_Unresolved:
			return true;

		case DataType::Int:
		case DataType::Float:
		case DataType::Bool:
		case DataType::Pattern:
		case DataType::Void:
		case DataType::CapId:
		case DataType::Cap:
		case DataType::_Marker:
		case DataType::_Scope:
		case DataType::_Color:
		default:
			return false;
		}
	}

	bool is_highlightable(DataType type)
	{
		switch (type)
		{
		case DataType::Int:
		case DataType::Match:
		case DataType::Matches:
		case DataType::IRange:
		case DataType::_Unresolved:
			return true;

		case DataType::Float:
		case DataType::Bool:
		case DataType::String:
		case DataType::Pattern:
		case DataType::Void:
		case DataType::CapId:
		case DataType::Cap:
		case DataType::SRange:
		case DataType::List:
		case DataType::_Marker:
		case DataType::_Scope:
		case DataType::_Color:
		default:
			return false;
		}
	}

	bool is_pageable(DataType type)
	{
		switch (type)
		{
		case DataType::String:
		case DataType::_Unresolved:
			return true;

		case DataType::Int:
		case DataType::Match:
		case DataType::Matches:
		case DataType::IRange:
		case DataType::Float:
		case DataType::Bool:
		case DataType::Pattern:
		case DataType::Void:
		case DataType::CapId:
		case DataType::Cap:
		case DataType::SRange:
		case DataType::List:
		case DataType::_Marker:
		case DataType::_Scope:
		case DataType::_Color:
		default:
			return false;
		}
	}

	BinaryOperator binary_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::And:
			return BinaryOperator::And;
		case TokenType::Asterisk:
			return BinaryOperator::Asterisk;
		case TokenType::Comma:
			return BinaryOperator::Comma;
		case TokenType::EqualTo:
			return BinaryOperator::EqualTo;
		case TokenType::Except:
			return BinaryOperator::Except;
		case TokenType::GreaterThan:
			return BinaryOperator::GreaterThan;
		case TokenType::GreaterThanOrEqualTo:
			return BinaryOperator::GreaterThanOrEqualTo;
		case TokenType::LessThan:
			return BinaryOperator::LessThan;
		case TokenType::LessThanOrEqualTo:
			return BinaryOperator::LessThanOrEqualTo;
		case TokenType::Minus:
			return BinaryOperator::Minus;
		case TokenType::Mod:
			return BinaryOperator::Mod;
		case TokenType::NotEqualTo:
			return BinaryOperator::NotEqualTo;
		case TokenType::Or:
			return BinaryOperator::Or;
		case TokenType::Plus:
			return BinaryOperator::Plus;
		case TokenType::Repeat:
			return BinaryOperator::Repeat;
		case TokenType::Slash:
			return BinaryOperator::Slash;
		case TokenType::To:
			return BinaryOperator::To;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	std::optional<DataType> evaltype(BinaryOperator op, DataType lhs, DataType rhs)
	{
		switch (op)
		{
		case BinaryOperator::And:
		case BinaryOperator::Or:
			if (can_cast_implicit(lhs, DataType::Bool) && can_cast_implicit(rhs, DataType::Bool))
				return DataType::Bool;
			break;

		case BinaryOperator::Asterisk:
		case BinaryOperator::Minus:
		case BinaryOperator::Mod:
		case BinaryOperator::Plus:
		case BinaryOperator::Slash:
			if (can_cast_implicit(lhs, DataType::Int) && can_cast_implicit(rhs, DataType::Int))
				return DataType::Int;
			else if ((can_cast_implicit(lhs, DataType::Int) || can_cast_implicit(lhs, DataType::Float)) && (can_cast_implicit(rhs, DataType::Int) || can_cast_implicit(rhs, DataType::Float)))
				return DataType::Float;
			break;
		
		case BinaryOperator::EqualTo:
		case BinaryOperator::NotEqualTo:
			if (can_cast_implicit(lhs, rhs) || can_cast_implicit(rhs, lhs))
				return DataType::Bool;
			break;
		
		case BinaryOperator::GreaterThan:
		case BinaryOperator::GreaterThanOrEqualTo:
		case BinaryOperator::LessThan:
		case BinaryOperator::LessThanOrEqualTo:
			if ((can_cast_implicit(lhs, DataType::Int) || can_cast_implicit(lhs, DataType::Float)) && (can_cast_implicit(rhs, DataType::Int) || can_cast_implicit(rhs, DataType::Float)))
				return DataType::Bool;
			break;

		case BinaryOperator::To:
			if (can_cast_implicit(lhs, DataType::Int) && can_cast_implicit(rhs, DataType::Int))
				return DataType::IRange;
			else if (can_cast_implicit(lhs, DataType::String) && can_cast_implicit(rhs, DataType::String))
				return DataType::SRange;
			break;

		case BinaryOperator::Comma:
		case BinaryOperator::Except:
		case BinaryOperator::Repeat:
			if (can_cast_implicit(lhs, DataType::Pattern) && can_cast_implicit(rhs, DataType::IRange))
				return DataType::Pattern;
			break;
		}

		return std::nullopt;
	}

	PrefixOperator prefix_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::Ahead:
			return PrefixOperator::Ahead;
		case TokenType::Behind:
			return PrefixOperator::Behind;
		case TokenType::Max:
			return PrefixOperator::Max;
		case TokenType::Min:
			return PrefixOperator::Min;
		case TokenType::Minus:
			return PrefixOperator::Minus;
		case TokenType::Not:
			return PrefixOperator::Not;
		case TokenType::NotAhead:
			return PrefixOperator::NotAhead;
		case TokenType::NotBehind:
			return PrefixOperator::NotBehind;
		case TokenType::Optional:
			return PrefixOperator::Optional;
		case TokenType::Ref:
			return PrefixOperator::Ref;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	std::optional<DataType> evaltype(PrefixOperator op, DataType type)
	{
		switch (op)
		{
		case PrefixOperator::Ahead:
		case PrefixOperator::Behind:
		case PrefixOperator::NotAhead:
		case PrefixOperator::NotBehind:
		case PrefixOperator::Optional:
		case PrefixOperator::Ref:
			if (can_cast_implicit(type, DataType::Pattern))
				return DataType::Pattern;
			break;

		case PrefixOperator::Max:
		case PrefixOperator::Min:
			if (can_cast_implicit(type, DataType::Int))
				return DataType::IRange;
			else if (can_cast_implicit(type, DataType::String))
				return DataType::SRange;
			break;

		case PrefixOperator::Minus:
			if (can_cast_implicit(type, DataType::Int) || can_cast_implicit(type, DataType::Float))
				return type;
			break;

		case PrefixOperator::Not:
			if (can_cast_implicit(type, DataType::Bool))
				return type;
			break;
		}

		return std::nullopt;
	}

	PatternSimpleRepeatOperator pattern_simple_repeat_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::Asterisk:
			return PatternSimpleRepeatOperator::Asterisk;
		case TokenType::Plus:
			return PatternSimpleRepeatOperator::Plus;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}
}
