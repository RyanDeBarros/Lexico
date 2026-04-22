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

	void Lexer::tokenize(const std::string_view script)
	{
		std::vector<Token> tokens;

		size_t line = 0;
		size_t column = 0;

		Token token{ .type = TokenType::EndOfFile, .lexeme = "", .line = 0, .column = 0 };

		auto start_token = [&token, line, column](TokenType type) {
			token.type = type;
			token.line = line;
			token.column = column;
			};

		auto add_token = [&token, &tokens]() {
			if (token.type != TokenType::EndOfFile)
			{
				// TODO if type is identifier, check if lexeme is a reserved keyword/type/bool then update type accordingly

				tokens.push_back(std::move(token));
				token.lexeme.clear();
				token.type = TokenType::EndOfFile;
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

				int j = i;
				int k;
				while (k = j + 1, k < script.size() && script[k] != '\n' && script[k] != '\r')
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
}
