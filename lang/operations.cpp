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

	std::string friendly_name(DataType type)
	{
		switch (type)
		{
		case DataType::Int:
			return "int";
		case DataType::Float:
			return "float";
		case DataType::Bool:
			return "bool";
		case DataType::String:
			return "string";
		case DataType::Void:
			return "void";
		case DataType::Pattern:
			return "pattern";
		case DataType::Match:
			return "match";
		case DataType::Matches:
			return "matches";
		case DataType::CapId:
			return "capid";
		case DataType::Cap:
			return "cap";
		case DataType::IRange:
			return "irange";
		case DataType::SRange:
			return "srange";
		case DataType::List:
			return "list";
		default:
			return "";
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

	bool can_cast(DataType from, DataType to)
	{
		if (to == DataType::Void)
			return true;

		switch (from)
		{
			case DataType::Int:
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
			default:
				return false;
		}
	}
}
