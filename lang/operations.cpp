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
		case TokenType::StringViewType:
			return DataType::StringView();
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
			if (lhs.can_cast_implicit(DataType::Bool()) && rhs.can_cast_implicit(DataType::Bool()))
				return DataType::Bool();

		case BinaryOperator::Or:
			if (lhs.can_cast_implicit(DataType::Bool()) && rhs.can_cast_implicit(DataType::Bool()))
				return DataType::Bool();
			else if (lhs.can_cast_implicit(DataType::Pattern()) && rhs.can_cast_implicit(DataType::Pattern()))
				return DataType::Pattern();
			break;

		case BinaryOperator::Assign:
			if (rhs.can_cast_implicit(lhs))
				return lhs;
			break;

		case BinaryOperator::Asterisk:
		case BinaryOperator::Minus:
		case BinaryOperator::Mod:
		case BinaryOperator::Plus:
		case BinaryOperator::Slash:
			if (lhs.can_cast_implicit(DataType::Int()) && rhs.can_cast_implicit(DataType::Int()))
				return DataType::Int();
			else if ((lhs.can_cast_implicit(DataType::Int()) || lhs.can_cast_implicit(DataType::Float()))
					&& (rhs.can_cast_implicit(DataType::Int()) || rhs.can_cast_implicit(DataType::Float())))
				return DataType::Float();
			break;
		
		case BinaryOperator::EqualTo:
		case BinaryOperator::NotEqualTo:
			if (lhs.can_cast_implicit(rhs) || rhs.can_cast_implicit(lhs))
				return DataType::Bool();
			break;
		
		case BinaryOperator::GreaterThan:
		case BinaryOperator::GreaterThanOrEqualTo:
		case BinaryOperator::LessThan:
		case BinaryOperator::LessThanOrEqualTo:
			if ((lhs.can_cast_implicit(DataType::Int()) || lhs.can_cast_implicit(DataType::Float()))
					&& (rhs.can_cast_implicit(DataType::Int()) || rhs.can_cast_implicit(DataType::Float())))
				return DataType::Bool();
			break;

		case BinaryOperator::To:
			if (lhs.can_cast_implicit(DataType::Int()) && rhs.can_cast_implicit(DataType::Int()))
				return DataType::IRange();
			else if (lhs.can_cast_implicit(DataType::String()) && rhs.can_cast_implicit(DataType::String()))
				return DataType::SRange();
			break;

		case BinaryOperator::Comma:
		case BinaryOperator::Except:
			if (lhs.can_cast_implicit(DataType::Pattern()) && rhs.can_cast_implicit(DataType::Pattern()))
				return DataType::Pattern();

		case BinaryOperator::Repeat:
			if (lhs.can_cast_implicit(DataType::Pattern()) && rhs.can_cast_implicit(DataType::IRange()))
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
			if (type.can_cast_implicit(DataType::Pattern()))
				return DataType::Pattern();
			break;

		case PrefixOperator::Ref:
			if (type.can_cast_implicit(DataType::CapId()))
				return DataType::Pattern();
			break;

		case PrefixOperator::Max:
		case PrefixOperator::Min:
			if (type.can_cast_implicit(DataType::Int()))
				return DataType::IRange();
			else if (type.can_cast_implicit(DataType::String()))
				return DataType::SRange();
			break;

		case PrefixOperator::Minus:
			if (type.can_cast_implicit(DataType::Int()) || type.can_cast_implicit(DataType::Float()))
				return type;
			break;

		case PrefixOperator::Not:
			if (type.can_cast_implicit(DataType::Bool()))
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
