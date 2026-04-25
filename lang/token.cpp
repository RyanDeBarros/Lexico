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

	std::string Token::line_number_prefix() const
	{
		std::string line_number = std::to_string(start_line) + ".";
		const unsigned int digit_count = line_number.length();
		for (unsigned int i = 0; i < 4 - digit_count % 4; ++i)
			line_number += " ";
		return line_number;
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
