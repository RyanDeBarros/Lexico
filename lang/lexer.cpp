#include "lexer.h"

namespace lx
{
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
		else if (token.lexeme == "log")
			return TokenType::Log;
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

		class ScriptPointer
		{
			unsigned int _line = 1;
			unsigned int _last_line = 1;
			unsigned int _column = 1;
			unsigned int _last_column = 1;

		public:
			void move_right(unsigned int n = 1)
			{
				for (unsigned int i = 0; i < n; ++i)
				{
					_last_column = _column;
					++_column;
				}
			}

			void move_down()
			{
				_last_line = _line;
				++_line;
				_last_column = _column;
				_column = 1;
			}

			unsigned int line() const
			{
				return _line;
			}

			unsigned int last_line() const
			{
				return _last_line;
			}

			unsigned int column() const
			{
				return _column;
			}

			unsigned int last_column() const
			{
				return _last_column;
			}
		};

		ScriptPointer ptr;
		Token token;

		auto start_token = [&token, &ptr](TokenType type) {
			token.type = type;
			token.start_line = ptr.line();
			token.start_column = ptr.column();
			};

		auto add_token = [&token, &tokens, &ptr]() {
			if (token.type != TokenType::EndOfFile)
			{
				token.type = resolve_identifier(token);
				token.end_line = ptr.last_line();
				token.end_column = ptr.last_column();
				tokens.push_back(std::move(token));
				token = {};
			}
			};

		for (size_t i = 0; i < script.size(); ++i)
		{
			const char c = script[i];

			if (token.type == TokenType::String)
			{
				if (c == '"')
				{
					add_token();
					ptr.move_right();
					continue;
				}

				if (c == '\\')
				{
					if (i + 1 < script.size())
					{
						if (script[i + 1] == '"')
						{
							token.lexeme += '"';
							++i;
							ptr.move_right(2);
							continue;
						}
						else if (script[i + 1] == '\\' && i + 2 < script.size() && script[i + 2] == '"')
						{
							token.lexeme += '\\';
							++i;
							ptr.move_right(2);
							continue;
						}
					}
				}

				token.lexeme += c;

				if (c == '\n' || c == '\r')
				{
					if (c == '\r' && i + 1 < script.size() && script[i + 1] == '\n')
						++i;

					ptr.move_down();
				}
				else
					ptr.move_right();
				continue;
			}

			if (c == '"')
			{
				add_token();  // add ongoing token
				start_token(TokenType::String);
				ptr.move_right();
				continue;
			}

			if (c == '\n' || c == '\r')
			{
				add_token();  // add ongoing token

				if (c == '\r' && i + 1 < script.size() && script[i + 1] == '\n')
					++i;

				// add newline token - don't add multiple consecutively or if at beginning of script
				if (!tokens.empty() && tokens.back().type != TokenType::Newline)
				{
					start_token(TokenType::Newline);
					add_token();
				}

				ptr.move_down();
				continue;
			}

			if (c == '#')
			{
				add_token();  // add ongoing token

				size_t j = i;
				while (j + 1 < script.size() && script[j + 1] != '\n' && script[j + 1] != '\r')
					++j;

				ptr.move_right(j - i);
				i = j;
				continue;
			}

			if (c == ',')
			{
				add_token();  // add ongoing token
				start_token(TokenType::Comma);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '\\')
			{
				add_token();  // add ongoing token
				start_token(TokenType::Runoff);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '%')
			{
				add_token();  // add ongoing token
				start_token(TokenType::Percent);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '$')
			{
				add_token();  // add ongoing token
				start_token(TokenType::BuiltinSymbol);
				token.lexeme = c;
				ptr.move_right();
				continue;
			}

			if (c == '(')
			{
				add_token();  // add ongoing token
				start_token(TokenType::LParen);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == ')')
			{
				add_token();  // add ongoing token
				start_token(TokenType::RParen);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '[')
			{
				add_token();  // add ongoing token
				start_token(TokenType::LBracket);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == ']')
			{
				add_token();  // add ongoing token
				start_token(TokenType::RBracket);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '+')
			{
				add_token();  // add ongoing token
				start_token(TokenType::Plus);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '-')
			{
				add_token();  // add ongoing token

				if (i + 1 < script.size() && script[i + 1] == '>')
				{
					start_token(TokenType::Arrow);
					add_token();
					++i;
					ptr.move_right();
				}
				else
				{
					start_token(TokenType::Minus);
					add_token();
				}

				ptr.move_right();
				continue;
			}

			if (c == '/')
			{
				add_token();  // add ongoing token
				start_token(TokenType::Slash);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '*')
			{
				add_token();  // add ongoing token
				start_token(TokenType::Asterisk);
				add_token();
				ptr.move_right();
				continue;
			}

			if (c == '!' && i + 1 < script.size() && script[i + 1] == '=')
			{
				add_token();  // add ongoing token
				start_token(TokenType::NotEqualTo);
				add_token();
				++i;
				ptr.move_right(2);
				continue;
			}

			if (c == '=')
			{
				add_token();  // add ongoing token
				
				if (i + 1 < script.size() && script[i + 1] == '=')
				{
					start_token(TokenType::EqualTo);
					add_token();
					++i;
					ptr.move_right();
				}
				else
				{
					start_token(TokenType::Assign);
					add_token();
				}

				ptr.move_right();
				continue;
			}

			if (c == '<')
			{
				add_token();  // add ongoing token

				if (i + 1 < script.size() && script[i + 1] == '=')
				{
					start_token(TokenType::LessThanOrEqualTo);
					add_token();
					++i;
					ptr.move_right();
				}
				else
				{
					start_token(TokenType::LessThan);
					add_token();
				}

				ptr.move_right();
				continue;
			}

			if (c == '>')
			{
				add_token();  // add ongoing token

				if (i + 1 < script.size() && script[i + 1] == '=')
				{
					start_token(TokenType::GreaterThanOrEqualTo);
					add_token();
					++i;
					ptr.move_right();
				}
				else
				{
					start_token(TokenType::GreaterThan);
					add_token();
				}

				ptr.move_right();
				continue;
			}

			if (c == '.')
			{
				if (token.type == TokenType::Integer)
				{
					token.type = TokenType::Float;
					token.lexeme += c;
				}
				else
				{
					add_token();  // add ongoing token

					if (i + 1 < script.size() && isdigit(script[i + 1]))
					{
						start_token(TokenType::Float);
						token.lexeme = c;
					}
					else
					{
						start_token(TokenType::Dot);
						add_token();
					}
				}

				ptr.move_right();
				continue;
			}

			if (c == ' ' || c == '\t')
			{
				add_token();  // add ongoing token
				ptr.move_right();
				continue;
			}

			if (isdigit(c))
			{
				if (token.type != TokenType::Identifier && token.type != TokenType::BuiltinSymbol
					&& token.type != TokenType::Integer && token.type != TokenType::Float)
				{
					add_token();  // add ongoing token
					start_token(TokenType::Integer);
				}
				token.lexeme += c;
				ptr.move_right();
				continue;
			}

			if (isalpha(c) || c == '_')
			{
				if (token.type == TokenType::EndOfFile)
				{
					add_token();  // add ongoing token
					start_token(TokenType::Identifier);
				}

				token.lexeme += c;
				ptr.move_right();
				continue;
			}

			add_token();
			ptr.move_right();
		}

		add_token();  // add ongoing token

		start_token(TokenType::EndOfFile);
		tokens.push_back(token);

		// cancel runoff + newline cancellation pairs
		std::vector<Token> final_tokens;
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			if (tokens[i].type == TokenType::Runoff)
			{
				if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Newline)
					++i;
				continue;
			}

			if (tokens[i].type == TokenType::Not)
			{
				if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Ahead)
				{
					Token token = std::move(tokens[i]);
					token.end_column = tokens[i + 1].end_column;
					token.end_line = tokens[i + 1].end_line;
					token.type = TokenType::NotAhead;
					final_tokens.push_back(std::move(token));
					++i;
					continue;
				}

				if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::Behind)
				{
					Token token = std::move(tokens[i]);
					token.end_column = tokens[i + 1].end_column;
					token.end_line = tokens[i + 1].end_line;
					token.type = TokenType::NotBehind;
					final_tokens.push_back(std::move(token));
					++i;
					continue;
				}
			}

			final_tokens.push_back(std::move(tokens[i]));
		}
		_stream.load(std::move(final_tokens));
	}

	const TokenStream& Lexer::stream() const
	{
		return _stream;
	}

	TokenStream& Lexer::stream()
	{
		return _stream;
	}

	const std::vector<std::string_view>& Lexer::script_lines() const
	{
		return _script_lines;
	}
}
