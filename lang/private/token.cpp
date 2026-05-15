#include "token.h"

#include "constants.h"

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
		return start_line >= 1 ? script_lines[start_line - 1] : (script_lines.empty() ? "" : script_lines.back());
	}

	std::string ScriptSegment::line_number_prefix() const
	{
		std::string line_number = std::to_string(start_line) + ".";
		const unsigned int digit_count = line_number.length();
		for (unsigned int i = 0; i < 4 - digit_count % 4; ++i)
			line_number += " ";
		return line_number;
	}

	std::string ScriptSegment::underline(unsigned int tabs) const
	{
		std::stringstream ss;

		while (tabs > 0)
		{
			ss << constants::SPACED_TAB;
			--tabs;
		}

		for (unsigned int i = 1; i < start_column; ++i)
			ss << ' ';

		ss << '^';

		unsigned int ec = end_column;
		if (start_line != end_line)
		{
			size_t line_length = script_lines[start_line].size();
			ec = line_length > 0 ? line_length - 1 : 0;
		}

		for (unsigned int i = start_column + 1; i <= ec; ++i)
			ss << '~';

		return ss.str();
	}

	std::string ScriptSegment::message(const std::string_view header) const
	{
		std::stringstream ss;
		ss << header;
		ss << ":\n";
		std::string line_number = line_number_prefix();
		unsigned int tabs = 1 + line_number.size() / 4;
		ss << constants::SPACED_TAB << line_number << first_line() << '\n' << underline(tabs);
		return ss.str();
	}

	std::string ScriptSegment::batch_message(const std::vector<ScriptSegment>& segments, const std::string_view header)
	{
		std::stringstream ss;
		ss << header;
		ss << ":";

		unsigned int max_tabs = 0;
		std::vector<std::string> line_numbers;

		for (const ScriptSegment& segment : segments)
		{
			std::string line_number = segment.line_number_prefix();
			const unsigned int tabs = 1 + line_number.size() / 4;
			max_tabs = std::max(max_tabs, tabs);
			line_numbers.push_back(std::move(line_number));
		}

		for (size_t i = 0; i < segments.size(); ++i)
		{
			const unsigned int tabs = 1 + line_numbers[i].size() / 4;
			ss << '\n' << constants::SPACED_TAB << line_numbers[i];
			for (unsigned int j = 0; j < max_tabs - tabs; ++j)
				ss << constants::SPACED_TAB;
			ss << segments[i].first_line();
		}

		return ss.str();
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

	Token::Token(const ScriptSegment& segment)
		: segment(segment)
	{
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

	Keyword Token::keyword() const
	{
		if (!_keyword)
		{
			_keyword = impl_keyword();
			if (!_keyword)
				_keyword = Keyword::_None;
		}
		return *_keyword;
	}

	void Token::set_keyword(Keyword kw)
	{
		_keyword = kw;
	}

	std::optional<Keyword> Token::impl_keyword() const
	{
		if (type != TokenType::Identifier)
			return std::nullopt;

		if (lexeme == "int")
			return Keyword::IntType;
		else if (lexeme == "float")
			return Keyword::FloatType;
		else if (lexeme == "bool")
			return Keyword::BoolType;
		else if (lexeme == "string")
			return Keyword::StringType;
		else if (lexeme == "string_view")
			return Keyword::StringViewType;
		else if (lexeme == "void")
			return Keyword::VoidType;
		else if (lexeme == "pattern")
			return Keyword::PatternType;
		else if (lexeme == "match")
			return Keyword::MatchType;
		else if (lexeme == "matches")
			return Keyword::MatchesType;
		else if (lexeme == "capid")
			return Keyword::CapIdType;
		else if (lexeme == "cap")
			return Keyword::CapType;
		else if (lexeme == "irange")
			return Keyword::IRangeType;
		else if (lexeme == "srange")
			return Keyword::SRangeType;
		else if (lexeme == "list")
			return Keyword::ListType;
		else if (lexeme == "and")
			return Keyword::And;
		else if (lexeme == "apply")
			return Keyword::Apply;
		else if (lexeme == "as")
			return Keyword::As;
		else if (lexeme == "break")
			return Keyword::Break;
		else if (lexeme == "color")
			return Keyword::Color;
		else if (lexeme == "continue")
			return Keyword::Continue;
		else if (lexeme == "delete")
			return Keyword::Delete;
		else if (lexeme == "elif")
			return Keyword::Elif;
		else if (lexeme == "else")
			return Keyword::Else;
		else if (lexeme == "end")
			return Keyword::End;
		else if (lexeme == "filter")
			return Keyword::Filter;
		else if (lexeme == "findall")
			return Keyword::FindAll;
		else if (lexeme == "fn")
			return Keyword::Fn;
		else if (lexeme == "for")
			return Keyword::For;
		else if (lexeme == "highlight")
			return Keyword::Highlight;
		else if (lexeme == "if")
			return Keyword::If;
		else if (lexeme == "in")
			return Keyword::In;
		else if (lexeme == "let")
			return Keyword::Let;
		else if (lexeme == "log")
			return Keyword::Log;
		else if (lexeme == "page")
			return Keyword::Page;
		else if (lexeme == "pop")
			return Keyword::Pop;
		else if (lexeme == "push")
			return Keyword::Push;
		else if (lexeme == "replace")
			return Keyword::Replace;
		else if (lexeme == "return")
			return Keyword::Return;
		else if (lexeme == "search")
			return Keyword::Search;
		else if (lexeme == "scope")
			return Keyword::Scope;
		else if (lexeme == "var")
			return Keyword::Var;
		else if (lexeme == "while")
			return Keyword::While;
		else if (lexeme == "with")
			return Keyword::With;
		else if (lexeme == "append")
			return Keyword::Append;
		else if (lexeme == "ahead")
			return Keyword::Ahead;
		else if (lexeme == "behind")
			return Keyword::Behind;
		else if (lexeme == "capture")
			return Keyword::Capture;
		else if (lexeme == "except")
			return Keyword::Except;
		else if (lexeme == "lazy")
			return Keyword::Lazy;
		else if (lexeme == "greedy")
			return Keyword::Greedy;
		else if (lexeme == "max")
			return Keyword::Max;
		else if (lexeme == "min")
			return Keyword::Min;
		else if (lexeme == "mod")
			return Keyword::Mod;
		else if (lexeme == "not")
			return Keyword::Not;
		else if (lexeme == "optional")
			return Keyword::Optional;
		else if (lexeme == "or")
			return Keyword::Or;
		else if (lexeme == "ref")
			return Keyword::Ref;
		else if (lexeme == "repeat")
			return Keyword::Repeat;
		else if (lexeme == "to")
			return Keyword::To;
		else
			return std::nullopt;
	}

	bool Token::is_datatype() const
	{
		switch (keyword())
		{
		case Keyword::IntType:
		case Keyword::FloatType:
		case Keyword::BoolType:
		case Keyword::StringType:
		case Keyword::StringViewType:
		case Keyword::VoidType:
		case Keyword::PatternType:
		case Keyword::MatchType:
		case Keyword::MatchesType:
		case Keyword::CapIdType:
		case Keyword::CapType:
		case Keyword::IRangeType:
		case Keyword::SRangeType:
		case Keyword::ListType:
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
		case TokenType::CapId:
			return true;
		default:
			return false;
		}
	}

	bool Token::is_binary_operator() const
	{
		switch (type)
		{
		case TokenType::Assign:
		case TokenType::Asterisk:
		case TokenType::Comma:
		case TokenType::EqualTo:
		case TokenType::GreaterThan:
		case TokenType::GreaterThanOrEqualTo:
		case TokenType::LessThan:
		case TokenType::LessThanOrEqualTo:
		case TokenType::Minus:
		case TokenType::NotEqualTo:
		case TokenType::Plus:
		case TokenType::Slash:
			return true;

		default:
			switch (keyword())
			{
			case Keyword::And:
			case Keyword::Except:
			case Keyword::Mod:
			case Keyword::Or:
			case Keyword::Repeat:
			case Keyword::To:
				return true;
			}

			return false;
		}
	}

	bool Token::is_prefix_operator() const
	{
		if (type == TokenType::Minus)
			return true;

		switch (keyword())
		{
		case Keyword::Ahead:
		case Keyword::Behind:
		case Keyword::Max:
		case Keyword::Min:
		case Keyword::Not:
		case Keyword::NotAhead:
		case Keyword::NotBehind:
		case Keyword::Optional:
		case Keyword::Ref:
			return true;

		default:
			return false;
		}
	}

	bool Token::is_postfix_operator() const
	{
		switch (type)
		{
		case TokenType::Asterisk:
		case TokenType::Plus:
			return true;
		}

		if (keyword() == Keyword::As)
			return true;

		return false;
	}

	Precedence Token::precedence() const
	{
		switch (type)
		{
		case TokenType::Asterisk:
		case TokenType::Slash:
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

		case TokenType::Comma:
			return Precedence::Comma;

		case TokenType::Assign:
			return Precedence::Assign;

		default:
		{
			switch (keyword())
			{
			case Keyword::Mod:
				return Precedence::Mult;

			case Keyword::And:
				return Precedence::And;

			case Keyword::Or:
				return Precedence::Or;

			case Keyword::To:
				return Precedence::To;

			case Keyword::Repeat:
				return Precedence::Repeat;

			case Keyword::Except:
				return Precedence::Except;
			}

			return Precedence::None;
		}
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
