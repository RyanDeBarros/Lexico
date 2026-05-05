#include "token.h"

#include <stdexcept>
#include <sstream>

namespace lx
{
	Precedence operator+(Precedence a, Precedence b)
	{
		return static_cast<Precedence>(static_cast<int>(a) + static_cast<int>(b));
	}

	Precedence operator+(Precedence a, int b)
	{
		return static_cast<Precedence>(static_cast<int>(a) + b);
	}

	ScriptSegment::ScriptSegment(const std::vector<std::string_view>& script_lines)
		: script_lines(script_lines)
	{
	}

	ScriptSegment::ScriptSegment(const ScriptSegment& other)
		: script_lines(other.script_lines), start_line(other.start_line), start_column(other.start_column), end_line(other.end_line), end_column(other.end_column)
	{
	}

	ScriptSegment::ScriptSegment(ScriptSegment&& other) noexcept
		: script_lines(other.script_lines), start_line(other.start_line), start_column(other.start_column), end_line(other.end_line), end_column(other.end_column)
	{
	}

	ScriptSegment& ScriptSegment::operator=(const ScriptSegment& other)
	{
		if (this != &other)
		{
			start_line = other.start_line;
			start_column = other.start_column;
			end_line = other.end_line;
			end_column = other.end_column;
		}
		return *this;
	}

	ScriptSegment& ScriptSegment::operator=(ScriptSegment&& other) noexcept
	{
		if (this != &other)
		{
			start_line = other.start_line;
			start_column = other.start_column;
			end_line = other.end_line;
			end_column = other.end_column;
		}
		return *this;
	}

	std::string_view ScriptSegment::first_line() const
	{
		return script_lines[start_line - 1];
	}

	std::string ScriptSegment::line_number_prefix() const
	{
		std::string line_number = std::to_string(start_line) + ".";
		const unsigned int digit_count = line_number.length();
		for (unsigned int i = 0; i < 4 - digit_count % 4; ++i)
			line_number += " ";
		return line_number;
	}

	ScriptSegment ScriptSegment::combined_right(ScriptSegment right) const
	{
		ScriptSegment combo = *this;
		if (right.end_line > end_line)
		{
			combo.end_column = right.end_column;
			combo.end_line = right.end_line;
		}
		else
			combo.end_column = std::max(end_column, right.end_column);
		return combo;
	}

	std::string Token::resolved() const
	{
		if (type != TokenType::String)
			return std::string(lexeme);

		std::string str;
		bool escaping = false;

		for (char c : lexeme.substr(1, lexeme.size() - 2))  // exclude ""s
		{
			if (escaping)
			{
				escaping = false;
				if (c == 'n')
					str += '\n';
				else if (c == 'r')
					str += '\r';
				else if (c == 't')
					str += '\t';
				else
				{
					if (c != '\\' && c != '"')
						str += '\\';
					str += c;
				}
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
		case TokenType::Assign:
		case TokenType::Asterisk:
		case TokenType::Comma:
		case TokenType::EqualTo:
		case TokenType::Except:
		case TokenType::GreaterThan:
		case TokenType::GreaterThanOrEqualTo:
		case TokenType::LessThan:
		case TokenType::LessThanOrEqualTo:
		case TokenType::Minus:
		case TokenType::Mod:
		case TokenType::NotEqualTo:
		case TokenType::Or:
		case TokenType::Plus:
		case TokenType::Repeat:
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
		case TokenType::Ahead:
		case TokenType::Behind:
		case TokenType::Max:
		case TokenType::Min:
		case TokenType::Minus:
		case TokenType::Not:
		case TokenType::NotAhead:
		case TokenType::NotBehind:
		case TokenType::Optional:
		case TokenType::Ref:
			return true;
		default:
			return false;
		}
	}

	bool Token::is_postfix_operator() const
	{
		switch (type)
		{
		case TokenType::As:
		case TokenType::Asterisk:
		case TokenType::Plus:
			return true;
		default:
			return false;
		}
	}

	Precedence Token::precedence() const
	{
		switch (type)
		{
		case TokenType::Asterisk:
		case TokenType::Slash:
		case TokenType::Mod:
			return Precedence::Mult;

		case TokenType::Plus:
		case TokenType::Minus:
			return Precedence::Sum;

		case TokenType::LessThan:
		case TokenType::LessThanOrEqualTo:
		case TokenType::GreaterThan:
		case TokenType::GreaterThanOrEqualTo:
			return Precedence::Compare;

		case TokenType::EqualTo:
		case TokenType::NotEqualTo:
			return Precedence::Equality;

		case TokenType::And:
			return Precedence::And;

		case TokenType::Or:
			return Precedence::Or;

		case TokenType::To:
			return Precedence::To;

		case TokenType::Repeat:
			return Precedence::Repeat;

		case TokenType::Except:
			return Precedence::Except;

		case TokenType::Comma:
			return Precedence::Comma;

		case TokenType::Assign:
			return Precedence::Assign;

		default:
			return Precedence::None;
		}
	}

	bool Token::is_right_associative() const
	{
		switch (type)
		{
		case TokenType::Assign:
			return true;
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
		return _ptr >= _tokens.size() || _tokens[_ptr].type == TokenType::EndOfFile;
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
