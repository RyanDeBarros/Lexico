#include "operations.h"

#include "errors.h"

#include <sstream>
#include <unordered_map>

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

	std::string friendly_name(DataType type)
	{
		switch (type)
		{
		case DataType::Int:
			return "'int'";
		case DataType::Float:
			return "'float'";
		case DataType::Bool:
			return "'bool'";
		case DataType::String:
			return "'string'";
		case DataType::Void:
			return "'void'";
		case DataType::Pattern:
			return "'pattern'";
		case DataType::Match:
			return "'match'";
		case DataType::Matches:
			return "'matches'";
		case DataType::CapId:
			return "'capid'";
		case DataType::Cap:
			return "'cap'";
		case DataType::IRange:
			return "'irange'";
		case DataType::SRange:
			return "'srange'";
		case DataType::List:
			return "'list'";
		case DataType::_Marker:
		case DataType::_Scope:
		case DataType::_Color:
			return "'symbol'";
		default:
			return "''";
		}
	}

	std::ostream& operator<<(std::ostream& os, DataType type)
	{
		return os << friendly_name(type);
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

	static const std::unordered_map<DataType, std::vector<MemberSignature>> MEMBER_TABLE = {
		{ DataType::String, {
			MemberSignature::make_data("len", DataType::Int),
			MemberSignature::make_method("[]", {
				{.return_type = DataType::String, .arg_types = { DataType::Int } },
				{.return_type = DataType::String, .arg_types = { DataType::IRange } },
			}),
		} },
		{ DataType::Match, {
			MemberSignature::make_data("caps", DataType::List),
			MemberSignature::make_data("start", DataType::Int),
			MemberSignature::make_data("end", DataType::Int),
			MemberSignature::make_data("len", DataType::Int),
			MemberSignature::make_data("pos", DataType::IRange),
			MemberSignature::make_data("str", DataType::String),
			MemberSignature::make_method("[]", { {.return_type = DataType::Cap, .arg_types = { DataType::Int } } }),
		} },
		{ DataType::Cap, {
			MemberSignature::make_data("exists", DataType::Bool),
			MemberSignature::make_data("start", DataType::Int),
			MemberSignature::make_data("end", DataType::Int),
			MemberSignature::make_data("len", DataType::Int),
			MemberSignature::make_data("pos", DataType::IRange),
			MemberSignature::make_data("str", DataType::String),
			MemberSignature::make_data("sub", DataType::Match),
		} },
		// TODO either 1. make list truly type mixing, in which case [] would return a new 'any'/'unresolved' type (it still can't change type dynamically, but it is an unknown type that should be casted when used). 2. make list templated, which introduces new syntax.
		{ DataType::List, {
			MemberSignature::make_data("len", DataType::Int),
		} },
	};

	const std::vector<MemberSignature>* data_type_members(DataType type)
	{
		auto it = MEMBER_TABLE.find(type);
		if (it != MEMBER_TABLE.end())
			return &it->second;
		else
			return nullptr;
	}

	bool can_cast(DataType from, DataType to)
	{
		if (to == DataType::Void || from == to)
			return true;

		switch (from)
		{
		case DataType::Int:
			return to == DataType::Float || to == DataType::Bool || to == DataType::String || to == DataType::Pattern || to == DataType::IRange;

		case DataType::Float:
		case DataType::Bool:
		case DataType::String:
			return to == DataType::Int || to == DataType::Float || to == DataType::Bool || to == DataType::String || to == DataType::Pattern;

		case DataType::Pattern:
			return to == DataType::String || to == DataType::Pattern;

		case DataType::Void:
		case DataType::Match:
		case DataType::Matches:
		case DataType::CapId:
		case DataType::Cap:
		case DataType::IRange:
		case DataType::SRange:
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

	StandardBinaryOperator standard_binary_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::And:
			return StandardBinaryOperator::And;
		case TokenType::Asterisk:
			return StandardBinaryOperator::Asterisk;
		case TokenType::EqualTo:
			return StandardBinaryOperator::EqualTo;
		case TokenType::GreaterThan:
			return StandardBinaryOperator::GreaterThan;
		case TokenType::GreaterThanOrEqualTo:
			return StandardBinaryOperator::GreaterThanOrEqualTo;
		case TokenType::LessThan:
			return StandardBinaryOperator::LessThan;
		case TokenType::LessThanOrEqualTo:
			return StandardBinaryOperator::LessThanOrEqualTo;
		case TokenType::Minus:
			return StandardBinaryOperator::Minus;
		case TokenType::Mod:
			return StandardBinaryOperator::Mod;
		case TokenType::NotEqualTo:
			return StandardBinaryOperator::NotEqualTo;
		case TokenType::Or:
			return StandardBinaryOperator::Or;
		case TokenType::Plus:
			return StandardBinaryOperator::Plus;
		case TokenType::Slash:
			return StandardBinaryOperator::Slash;
		case TokenType::To:
			return StandardBinaryOperator::To;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	std::optional<DataType> evaltype(StandardBinaryOperator op, DataType lhs, DataType rhs)
	{
		switch (op)
		{
		case StandardBinaryOperator::And:
		case StandardBinaryOperator::Or:
			if (lhs == DataType::Bool && rhs == DataType::Bool)
				return DataType::Bool;
			break;

		case StandardBinaryOperator::Asterisk:
		case StandardBinaryOperator::Minus:
		case StandardBinaryOperator::Mod:
		case StandardBinaryOperator::Plus:
		case StandardBinaryOperator::Slash:
			if (lhs == DataType::Int && rhs == DataType::Int)
				return DataType::Int;
			else if ((lhs == DataType::Int || lhs == DataType::Float) && (rhs == DataType::Int || rhs == DataType::Float))
				return DataType::Float;
			break;
		
		case StandardBinaryOperator::EqualTo:
		case StandardBinaryOperator::NotEqualTo:
			if (lhs == rhs && lhs != DataType::Void)
				return DataType::Bool;
			break;
		
		case StandardBinaryOperator::GreaterThan:
		case StandardBinaryOperator::GreaterThanOrEqualTo:
		case StandardBinaryOperator::LessThan:
		case StandardBinaryOperator::LessThanOrEqualTo:
			if ((lhs == DataType::Int || lhs == DataType::Float) && (rhs == DataType::Int || rhs == DataType::Float))
				return DataType::Bool;
			break;

		case StandardBinaryOperator::To:
			if (lhs == DataType::Int && rhs == DataType::Int)
				return DataType::IRange;
			else if (lhs == DataType::String && rhs == DataType::String)
				return DataType::SRange;
			break;
		}

		return std::nullopt;
	}

	StandardPrefixOperator standard_prefix_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::Max:
			return StandardPrefixOperator::Max;
		case TokenType::Min:
			return StandardPrefixOperator::Min;
		case TokenType::Minus:
			return StandardPrefixOperator::Minus;
		case TokenType::Not:
			return StandardPrefixOperator::Not;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	std::optional<DataType> evaltype(StandardPrefixOperator op, DataType type)
	{
		switch (op)
		{
		case StandardPrefixOperator::Max:
		case StandardPrefixOperator::Min:
			if (type == DataType::Int)
				return DataType::IRange;
			else if (type == DataType::String)
				return DataType::SRange;
			break;

		case StandardPrefixOperator::Minus:
			if (type == DataType::Int || type == DataType::Float)
				return type;
			break;

		case StandardPrefixOperator::Not:
			if (type == DataType::Bool)
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

	PatternPrefixOperator pattern_prefix_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::Ahead:
			return PatternPrefixOperator::Ahead;
		case TokenType::Behind:
			return PatternPrefixOperator::Behind;
		case TokenType::Max:
			return PatternPrefixOperator::Max;
		case TokenType::Min:
			return PatternPrefixOperator::Min;
		case TokenType::Not:
			return PatternPrefixOperator::Not;
		case TokenType::NotAhead:
			return PatternPrefixOperator::NotAhead;
		case TokenType::NotBehind:
			return PatternPrefixOperator::NotBehind;
		case TokenType::Optional:
			return PatternPrefixOperator::Optional;
		case TokenType::Ref:
			return PatternPrefixOperator::Ref;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	PatternBinaryOperator pattern_binary_operator(TokenType type)
	{
		switch (type)
		{
		case TokenType::Comma:
			return PatternBinaryOperator::Comma;
		case TokenType::Except:
			return PatternBinaryOperator::Except;
		case TokenType::Or:
			return PatternBinaryOperator::Or;
		case TokenType::Repeat:
			return PatternBinaryOperator::Repeat;
		case TokenType::To:
			return PatternBinaryOperator::To;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot convert token type " << static_cast<int>(type);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}
}
