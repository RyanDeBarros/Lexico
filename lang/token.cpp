#include "token.h"

#include <stdexcept>
#include <sstream>

namespace lx
{
	std::string Token::resolved() const
	{
		std::string str;
		bool escaping = false;

		for (char c : lexeme)
		{
			if (escaping)
			{
				escaping = false;
				if (c != '\\' && c != '"')
					str += '\\';
				str += c;
			}
			else if (c == '\\')
				escaping = true;
			else
				str += c;
		}

		if (escaping)
			str += '\\';

		return str;
	}

	std::string Token::line_number_prefix() const
	{
		std::string line_number = std::to_string(start_line) + ".";
		const unsigned int digit_count = line_number.length();
		for (unsigned int i = 0; i < 4 - digit_count % 4; ++i)
			line_number += " ";
		return line_number;
	}

	bool Token::is_datatype() const
	{
		switch (type)
		{
		case TokenType::IntType:
		case TokenType::FloatType:
		case TokenType::BoolType:
		case TokenType::StringType:
		case TokenType::VoidType:
		case TokenType::PatternType:
		case TokenType::MatchType:
		case TokenType::MatchesType:
		case TokenType::CapIdType:
		case TokenType::CapType:
		case TokenType::IRangeType:
		case TokenType::SRangeType:
		case TokenType::ListType:
			return true;
		default:
			return false;
		}
	}

	bool Token::is_literal() const
	{
		switch (type)
		{
		case TokenType::Integer:
		case TokenType::Float:
		case TokenType::String:
		case TokenType::Bool:
			return true;
		default:
			return false;
		}
	}

	bool Token::is_binary_operator() const
	{
		switch (type)
		{
		case TokenType::And:
		case TokenType::Asterisk:
		case TokenType::Dot:
		case TokenType::EqualTo:
		case TokenType::GreaterThan:
		case TokenType::GreaterThanOrEqualTo:
		case TokenType::LessThan:
		case TokenType::LessThanOrEqualTo:
		case TokenType::Minus:
		case TokenType::Mod:
		case TokenType::NotEqualTo:
		case TokenType::Or:
		case TokenType::Plus:
		case TokenType::Slash:
		case TokenType::To:
			return true;
		default:
			return false;
		}
	}

	bool Token::is_prefix_operator() const
	{
		switch (type)
		{
		case TokenType::Max:
		case TokenType::Min:
		case TokenType::Minus:
		case TokenType::Not:
			return true;
		default:
			return false;
		}
	}

	int Token::precedence() const
	{
		switch (type)
		{
		case TokenType::Dot:
			return 8;

		case TokenType::Asterisk:
		case TokenType::Slash:
		case TokenType::Mod:
			return 7;

		case TokenType::Plus:
		case TokenType::Minus:
			return 6;

		case TokenType::To:
			return 5;

		case TokenType::LessThan:
		case TokenType::LessThanOrEqualTo:
		case TokenType::GreaterThan:
		case TokenType::GreaterThanOrEqualTo:
			return 4;

		case TokenType::EqualTo:
		case TokenType::NotEqualTo:
			return 3;

		case TokenType::And:
			return 2;

		case TokenType::Or:
			return 1;

		default:
			return -1;
		}
	}

	bool Token::is_right_associative() const
	{
		switch (type)
		{
		default:
			return false;
		}
	}

	void TokenStream::load(std::vector<Token>&& tokens)
	{
		_tokens = std::move(tokens);
		_ptr = 0;
	}

	bool TokenStream::eof() const
	{
		return _ptr >= _tokens.size();
	}

	void TokenStream::advance(size_t n)
	{
		if (n > tokens_left())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot advance by " << n << " - only " << tokens_left() << " token(s) left";
			throw std::out_of_range(ss.str());
		}

		_ptr += n;
	}

	const Token& TokenStream::peek(size_t n) const
	{
		if (n >= tokens_left())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot peek by " << n << " - only " << tokens_left() << " token(s) left";
			throw std::out_of_range(ss.str());
		}

		return _tokens[_ptr + n];
	}

	Token& TokenStream::ref(size_t n)
	{
		if (n >= tokens_left())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot ref by " << n << " - only " << tokens_left() << " token(s) left";
			throw std::out_of_range(ss.str());
		}

		return _tokens[_ptr + n];
	}

	void TokenStream::seek(size_t i)
	{
		if (i > _tokens.size())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot seek to position " << i << " - out of range " << _tokens.size();
			throw std::out_of_range(ss.str());
		}

		_ptr = i;
	}

	size_t TokenStream::tokens_left() const
	{
		return _ptr < _tokens.size() ? _tokens.size() - _ptr : 0;
	}
}
