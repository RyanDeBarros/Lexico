#include "lexer.h"

#include <stdexcept>
#include <sstream>

namespace lx
{
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
		if (n >= tokens_left())
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

	size_t TokenStream::tokens_left() const
	{
		return _ptr < _tokens.size() ? _tokens.size() - _ptr : 0;
	}

	static TokenType resolve_identifier(const Token& token)
	{
		if (token.type != TokenType::Identifier)
			return token.type;

		if (token.lexeme == "true")
			return TokenType::Bool;
		else if (token.lexeme == "false")
			return TokenType::Bool;
		else if (token.lexeme == "int")
			return TokenType::IntType;
		else if (token.lexeme == "float")
			return TokenType::FloatType;
		else if (token.lexeme == "bool")
			return TokenType::BoolType;
		else if (token.lexeme == "string")
			return TokenType::StringType;
		else if (token.lexeme == "void")
			return TokenType::VoidType;
		else if (token.lexeme == "pattern")
			return TokenType::PatternType;
		else if (token.lexeme == "match")
			return TokenType::MatchType;
		else if (token.lexeme == "matches")
			return TokenType::MatchesType;
		else if (token.lexeme == "capid")
			return TokenType::CapIdType;
		else if (token.lexeme == "cap")
			return TokenType::CapType;
		else if (token.lexeme == "irange")
			return TokenType::IRangeType;
		else if (token.lexeme == "srange")
			return TokenType::SRangeType;
		else if (token.lexeme == "list")
			return TokenType::ListType;
		else if (token.lexeme == "and")
			return TokenType::And;
		else if (token.lexeme == "apply")
			return TokenType::Apply;
		else if (token.lexeme == "as")
			return TokenType::As;
		else if (token.lexeme == "break")
			return TokenType::Break;
		else if (token.lexeme == "color")
			return TokenType::Color;
		else if (token.lexeme == "continue")
			return TokenType::Continue;
		else if (token.lexeme == "delete")
			return TokenType::Delete;
		else if (token.lexeme == "elif")
			return TokenType::Elif;
		else if (token.lexeme == "else")
			return TokenType::Else;
		else if (token.lexeme == "end")
			return TokenType::End;
		else if (token.lexeme == "filter")
			return TokenType::Filter;
		else if (token.lexeme == "find")
			return TokenType::Find;
		else if (token.lexeme == "fn")
			return TokenType::Fn;
		else if (token.lexeme == "highlight")
			return TokenType::Highlight;
		else if (token.lexeme == "if")
			return TokenType::If;
		else if (token.lexeme == "in")
			return TokenType::In;
		else if (token.lexeme == "let")
			return TokenType::Let;
		else if (token.lexeme == "page")
			return TokenType::Page;
		else if (token.lexeme == "pop")
			return TokenType::Pop;
		else if (token.lexeme == "push")
			return TokenType::Push;
		else if (token.lexeme == "replace")
			return TokenType::Replace;
		else if (token.lexeme == "return")
			return TokenType::Return;
		else if (token.lexeme == "scope")
			return TokenType::Scope;
		else if (token.lexeme == "var")
			return TokenType::Var;
		else if (token.lexeme == "while")
			return TokenType::While;
		else if (token.lexeme == "with")
			return TokenType::With;
		else if (token.lexeme == "append")
			return TokenType::Append;
		else if (token.lexeme == "ahead")
			return TokenType::Ahead;
		else if (token.lexeme == "behind")
			return TokenType::Behind;
		else if (token.lexeme == "capture")
			return TokenType::Capture;
		else if (token.lexeme == "except")
			return TokenType::Except;
		else if (token.lexeme == "lazy")
			return TokenType::Lazy;
		else if (token.lexeme == "max")
			return TokenType::Max;
		else if (token.lexeme == "min")
			return TokenType::Min;
		else if (token.lexeme == "mod")
			return TokenType::Mod;
		else if (token.lexeme == "not")
			return TokenType::Not;
		else if (token.lexeme == "optional")
			return TokenType::Optional;
		else if (token.lexeme == "or")
			return TokenType::Or;
		else if (token.lexeme == "ref")
			return TokenType::Ref;
		else if (token.lexeme == "repeat")
			return TokenType::Repeat;
		else if (token.lexeme == "to")
			return TokenType::To;
		else
			return TokenType::Identifier;
	}

	void Lexer::tokenize(const std::string_view script)
	{
		std::vector<Token> tokens;

		size_t line = 0;
		size_t column = 0;

		Token token;

		auto start_token = [&token, line, column](TokenType type) {
			token.type = type;
			token.line = line;
			token.column = column;
			};

		auto add_token = [&token, &tokens]() {
			if (token.type != TokenType::EndOfFile)
			{
				token.type = resolve_identifier(token);
				tokens.push_back(std::move(token));
				token = {};
			}
			};

		for (size_t i = 0; i < script.size(); ++i)
		{
			const char c = script[i];

			if (c == '\n' || c == '\r')
			{
				if (c == '\r' && i + 1 < script.size() && script[i + 1] == '\n')
					++i;

				add_token(); // add ongoing token

				// add newline token - don't add multiple consecutively or if at beginning of script
				if (!tokens.empty() && tokens.back().type != TokenType::Newline)
				{
					start_token(TokenType::Newline);
					add_token();
				}

				column = 0;
				++line;

				continue;
			}

			if (c == '#')
			{
				add_token(); // add ongoing token

				size_t j = i;
				while (j + 1 < script.size() && script[j + 1] != '\n' && script[j + 1] != '\r')
					++j;

				column += j - i;
				i = j;
				continue;
			}

			// TODO tokenize

			++column;
		}

		add_token();
		tokens.push_back(token); // EOF token

		// TODO remove cancellation (RUNOFF, NEWLINE) pairs before loading tokens

		_stream.load(std::move(tokens));
	}

	const TokenStream& Lexer::stream() const
	{
		return _stream;
	}

	TokenStream& Lexer::stream()
	{
		return _stream;
	}
}
