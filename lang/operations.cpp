#include "operations.h"

#include "errors.h"
#include "constants.h"

#include <sstream>
#include <unordered_map>

namespace lx
{
	DataType data_type(TokenType type, const std::vector<TokenType>& underlying_types)
	{
		switch (type)
		{
		case TokenType::IntType:
			return DataType::Int();
		case TokenType::FloatType:
			return DataType::Float();
		case TokenType::BoolType:
			return DataType::Bool();
		case TokenType::StringType:
			return DataType::String();
		case TokenType::VoidType:
			return DataType::Void();
		case TokenType::PatternType:
			return DataType::Pattern();
		case TokenType::MatchType:
			return DataType::Match();
		case TokenType::MatchesType:
			return DataType::Matches();
		case TokenType::CapIdType:
			return DataType::CapId();
		case TokenType::CapType:
			return DataType::Cap();
		case TokenType::IRangeType:
			return DataType::IRange();
		case TokenType::SRangeType:
			return DataType::SRange();
		case TokenType::ListType:
		{
			if (underlying_types.empty())
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": cannot list token without underlying types";
				throw LxError(ErrorType::Internal, ss.str());
			}
			std::vector<TokenType> rest;
			for (size_t i = 1; i < underlying_types.size(); ++i)
				rest.push_back(underlying_types[i]);
			return DataType::List(data_type(underlying_types[0], rest));
		}
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
			return DataType::Int();
		case TokenType::Float:
			return DataType::Float();
		case TokenType::String:
			return DataType::String();
		case TokenType::Bool:
			return DataType::Bool();
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": literal type " << static_cast<int>(type) << " is not convertible to data type";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	bool can_cast_implicit(const DataType& from, const DataType& to)
	{
		if (to.simple() == SimpleType::Void || from == to)
			return true;

		switch (from.simple())
		{
		case SimpleType::Int:
			return to.simple() == SimpleType::Float || to.simple() == SimpleType::Bool || to.simple() == SimpleType::IRange;

		case SimpleType::Float:
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Bool;

		case SimpleType::Bool:
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Float;

		case SimpleType::String:
			return to.simple() == SimpleType::Pattern;

		case SimpleType::SRange:
			return to.simple() == SimpleType::Pattern;

		default:
			return false;
		}
	}

	bool can_cast_explicit(const DataType& from, const DataType& to)
	{
		if (can_cast_implicit(from, to))
			return true;

		switch (from.simple())
		{
		case SimpleType::Int:
		case SimpleType::Float:
			return to.simple() == SimpleType::String || to.simple() == SimpleType::Pattern;

		case SimpleType::Bool:
			return to.simple() == SimpleType::String;

		case SimpleType::String:
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Float || to.simple() == SimpleType::Bool;

		case SimpleType::SRange:
			return to.simple() == SimpleType::String;

		default:
			return false;
		}
	}

	bool is_iterable(const DataType& type)
	{
		switch (type.simple())
		{
		case SimpleType::String:
		case SimpleType::Matches:
		case SimpleType::Match:
		case SimpleType::IRange:
		case SimpleType::SRange:
		case SimpleType::List:
			return true;

		default:
			return false;
		}
	}

	bool is_highlightable(const DataType& type)
	{
		switch (type.simple())
		{
		case SimpleType::Int:
		case SimpleType::Match:
		case SimpleType::Matches:
		case SimpleType::IRange:
			return true;

		default:
			return false;
		}
	}

	bool is_pageable(const DataType& type)
	{
		switch (type.simple())
		{
		case SimpleType::String:
			return true;

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
		case TokenType::Assign:
			return BinaryOperator::Assign;
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

	std::optional<DataType> evaltype(BinaryOperator op, const DataType& lhs, const DataType& rhs)
	{
		switch (op)
		{
		case BinaryOperator::And:
		case BinaryOperator::Or:
			if (can_cast_implicit(lhs, DataType::Bool()) && can_cast_implicit(rhs, DataType::Bool()))
				return DataType::Bool();
			break;

		case BinaryOperator::Assign:
			if (can_cast_implicit(rhs, lhs))
				return lhs;
			break;

		case BinaryOperator::Asterisk:
		case BinaryOperator::Minus:
		case BinaryOperator::Mod:
		case BinaryOperator::Plus:
		case BinaryOperator::Slash:
			if (can_cast_implicit(lhs, DataType::Int()) && can_cast_implicit(rhs, DataType::Int()))
				return DataType::Int();
			else if ((can_cast_implicit(lhs, DataType::Int()) || can_cast_implicit(lhs, DataType::Float()))
					&& (can_cast_implicit(rhs, DataType::Int()) || can_cast_implicit(rhs, DataType::Float())))
				return DataType::Float();
			break;
		
		case BinaryOperator::EqualTo:
		case BinaryOperator::NotEqualTo:
			if (can_cast_implicit(lhs, rhs) || can_cast_implicit(rhs, lhs))
				return DataType::Bool();
			break;
		
		case BinaryOperator::GreaterThan:
		case BinaryOperator::GreaterThanOrEqualTo:
		case BinaryOperator::LessThan:
		case BinaryOperator::LessThanOrEqualTo:
			if ((can_cast_implicit(lhs, DataType::Int()) || can_cast_implicit(lhs, DataType::Float()))
					&& (can_cast_implicit(rhs, DataType::Int()) || can_cast_implicit(rhs, DataType::Float())))
				return DataType::Bool();
			break;

		case BinaryOperator::To:
			if (can_cast_implicit(lhs, DataType::Int()) && can_cast_implicit(rhs, DataType::Int()))
				return DataType::IRange();
			else if (can_cast_implicit(lhs, DataType::String()) && can_cast_implicit(rhs, DataType::String()))
				return DataType::SRange();
			break;

		case BinaryOperator::Comma:
		case BinaryOperator::Except:
		case BinaryOperator::Repeat:
			if (can_cast_implicit(lhs, DataType::Pattern()) && can_cast_implicit(rhs, DataType::IRange()))
				return DataType::Pattern();
			break;
		}

		return std::nullopt;
	}

	bool is_imperative(BinaryOperator op)
	{
		return op == BinaryOperator::Assign;
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

	std::optional<DataType> evaltype(PrefixOperator op, const DataType& type)
	{
		switch (op)
		{
		case PrefixOperator::Ahead:
		case PrefixOperator::Behind:
		case PrefixOperator::NotAhead:
		case PrefixOperator::NotBehind:
		case PrefixOperator::Optional:
		case PrefixOperator::Ref:
			if (can_cast_implicit(type, DataType::Pattern()))
				return DataType::Pattern();
			break;

		case PrefixOperator::Max:
		case PrefixOperator::Min:
			if (can_cast_implicit(type, DataType::Int()))
				return DataType::IRange();
			else if (can_cast_implicit(type, DataType::String()))
				return DataType::SRange();
			break;

		case PrefixOperator::Minus:
			if (can_cast_implicit(type, DataType::Int()) || can_cast_implicit(type, DataType::Float()))
				return type;
			break;

		case PrefixOperator::Not:
			if (can_cast_implicit(type, DataType::Bool()))
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
