#include "lexer.h"

namespace lx
{
	class ScriptPointer
	{
		unsigned int _line = 1;
		unsigned int _last_line = 1;
		unsigned int _column = 1;
		unsigned int _last_column = 1;
		unsigned int _index = 0;

	public:
		void move_right(unsigned int n = 1)
		{
			for (unsigned int i = 0; i < n; ++i)
			{
				_last_column = _column;
				++_column;
			}

			_last_line = _line;
			_index += n;
		}

		void move_down()
		{
			_last_line = _line;
			++_line;
			_last_column = _column;
			_column = 1;
			++_index;
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

		unsigned int index() const
		{
			return _index;
		}
	};

	class Tokenizer
	{
		const std::string_view _script;
		std::vector<Token>& _tokens;
		const std::vector<std::string_view>& _script_lines;
		std::vector<LxError>& _errors;
		ScriptPointer _ptr;
		unsigned int _str_offset = 0;
		Token _token;
		char _c;

	public:
		Tokenizer(const std::string_view script, std::vector<Token>& tokens, const std::vector<std::string_view>& _script_lines, std::vector<LxError>& errors)
			: _script(script), _tokens(tokens), _script_lines(_script_lines), _errors(errors), _token(new_token())
		{
			for (size_t i = 0; i < script.size(); ++i)
			{
				_c = script[i];

				if (_token.type == TokenType::String)
				{
					if (_c == '"')
					{
						_ptr.move_right();
						add_token();
						continue;
					}

					if (_c == '\\')
					{
						if (i + 1 < script.size())
						{
							if (script[i + 1] == '"')
							{
								++i;
								_ptr.move_right(2);
								continue;
							}
							else if (script[i + 1] == '\\' && i + 2 < script.size() && script[i + 2] == '"')
							{
								++i;
								_ptr.move_right(2);
								continue;
							}
						}
					}

					if (_c == '\n' || _c == '\r')
					{
						if (_c == '\r' && i + 1 < script.size() && script[i + 1] == '\n')
							++i;

						_ptr.move_down();
					}
					else
						_ptr.move_right();
					continue;
				}

				if (_c == '"')
				{
					add_token();  // add ongoing token
					start_token(TokenType::String);
					_ptr.move_right();
					continue;
				}

				if (_c == '\n' || _c == '\r')
				{
					add_token();  // add ongoing token

					if (_c == '\r' && i + 1 < script.size() && script[i + 1] == '\n')
						++i;

					// add newline token - don't add multiple consecutively or if at beginning of script
					if (!tokens.empty() && tokens.back().type != TokenType::Newline)
					{
						start_token(TokenType::Newline);
						add_token();
					}

					_ptr.move_down();
					continue;
				}

				if (_c == '#')
				{
					add_token();  // add ongoing token

					size_t j = i;
					while (j + 1 < script.size() && script[j + 1] != '\n' && script[j + 1] != '\r')
						++j;

					_ptr.move_right(j - i);
					i = j;
					continue;
				}

				if (_c == '$')
				{
					add_token();  // add ongoing token
					start_token(TokenType::BuiltinSymbol);
					_ptr.move_right();
					continue;
				}

				if (_token.type == TokenType::EndOfFile && _c == '!')
				{
					start_token(TokenType::CapId);
					_ptr.move_right();
					continue;
				}

				bool tokenized_char =
					tokenize_char(',', TokenType::Comma) ||
					tokenize_char('\\', TokenType::Runoff) ||
					tokenize_char('%', TokenType::Percent) ||
					tokenize_char('(', TokenType::LParen) ||
					tokenize_char(')', TokenType::RParen) ||
					tokenize_char('[', TokenType::LBracket) ||
					tokenize_char(']', TokenType::RBracket) ||
					tokenize_char('+', TokenType::Plus) ||
					tokenize_char('/', TokenType::Slash) ||
					tokenize_char('*', TokenType::Asterisk) ||
					tokenize_char_combo('-', '>', i, TokenType::Minus, TokenType::Arrow) ||
					tokenize_char_combo('=', '=', i, TokenType::Assign, TokenType::EqualTo) ||
					tokenize_char_combo('<', '=', i, TokenType::LessThan, TokenType::LessThanOrEqualTo) ||
					tokenize_char_combo('>', '=', i, TokenType::GreaterThan, TokenType::GreaterThanOrEqualTo) ||
					tokenize_double_char('!', '=', i, TokenType::NotEqualTo);

				if (tokenized_char)
					continue;

				if (_c == '.')
				{
					if (_token.type == TokenType::Integer)
						_token.type = TokenType::Float;
					else
					{
						add_token();  // add ongoing token

						if (i + 1 < script.size() && isdigit(script[i + 1]))
							start_token(TokenType::Float);
						else
						{
							start_token(TokenType::Dot);
							add_token();
						}
					}

					_ptr.move_right();
					continue;
				}

				if (_c == ' ' || _c == '\t')
				{
					add_token();  // add ongoing token
					_ptr.move_right();
					continue;
				}

				if (isdigit(_c))
				{
					if (_token.type != TokenType::Identifier && _token.type != TokenType::BuiltinSymbol
						&& _token.type != TokenType::Integer && _token.type != TokenType::Float && _token.type != TokenType::CapId)
					{
						add_token();  // add ongoing token
						start_token(TokenType::Integer);
					}
					_ptr.move_right();
					continue;
				}

				if (isalpha(_c) || _c == '_')
				{
					if (_token.type == TokenType::EndOfFile || _token.type == TokenType::Integer || _token.type == TokenType::Float)
					{
						add_token();  // add ongoing token
						start_token(TokenType::Identifier);
					}

					_ptr.move_right();
					continue;
				}

				add_token();
				_ptr.move_right();

				ScriptSegment error_segment = _token.segment;
				error_segment.end_line = _ptr.last_line();
				error_segment.end_column = _ptr.last_column();
				_errors.push_back(LxError::segment_error(error_segment, ErrorType::Syntax, "unrecognized token"));
			}

			add_token();  // add ongoing token

			start_token(TokenType::EndOfFile);
			impl_add_token();
			concat_runoffs();
		}

		Token new_token() const
		{
			return Token(ScriptSegment(_script_lines));
		}

		void start_token(TokenType type)
		{
			_token.type = type;
			_token.segment.start_line = _ptr.line();
			_token.segment.start_column = _ptr.column();
			_str_offset = _ptr.index();
		}

		void add_token()
		{
			if (_token.type != TokenType::EndOfFile)
				impl_add_token();
		}

		void impl_add_token()
		{
			_token.segment.end_line = _ptr.last_line();
			_token.segment.end_column = _ptr.last_column();

			_token.lexeme = _script.substr(_str_offset, _ptr.index() - _str_offset);
			if (_token.type == TokenType::Identifier)
			{
				if (_token.lexeme == "true" || _token.lexeme == "false")
					_token.type = TokenType::Bool;
			}

			_tokens.push_back(std::move(_token));
			
			_token = new_token();
			_str_offset = _ptr.index();
		}

		bool tokenize_char(char chr, TokenType type)
		{
			if (_c == chr)
			{
				add_token();  // add ongoing token
				start_token(type);
				_ptr.move_right();
				add_token();
				return true;
			}
			else
				return false;
		}

		bool tokenize_char_combo(char chr1, char chr2, size_t& i, TokenType single, TokenType combo)
		{
			if (_c == chr1)
			{
				add_token();  // add ongoing token

				if (i + 1 < _script.size() && _script[i + 1] == chr2)
				{
					start_token(combo);
					_ptr.move_right(2);
					add_token();
					++i;
				}
				else
				{
					start_token(single);
					_ptr.move_right();
					add_token();
				}

				return true;
			}
			else
				return false;
		}

		bool tokenize_double_char(char chr1, char chr2, size_t& i, TokenType type)
		{
			if (_c == chr1 && i + 1 < _script.size() && _script[i + 1] == chr2)
			{
				add_token();  // add ongoing token
				start_token(type);
				add_token();
				++i;
				_ptr.move_right(2);
				return true;
			}
			else
				return false;
		}

		void concat_runoffs()
		{
			std::vector<Token> final_tokens;
			for (size_t i = 0; i < _tokens.size(); ++i)
			{
				if (_tokens[i].type == TokenType::Runoff)
				{
					if (i + 1 < _tokens.size() && _tokens[i + 1].type == TokenType::Newline)
						++i;
					continue;
				}

				if (_tokens[i].keyword() == Keyword::Not)
				{
					if (i + 1 < _tokens.size() && _tokens[i + 1].keyword() == Keyword::Ahead)
					{
						_token = std::move(_tokens[i]);
						_token.segment.end_column = _tokens[i + 1].segment.end_column;
						_token.segment.end_line = _tokens[i + 1].segment.end_line;
						_token.set_keyword(Keyword::NotAhead);
						final_tokens.push_back(std::move(_token));
						++i;
						continue;
					}

					if (i + 1 < _tokens.size() && _tokens[i + 1].keyword() == Keyword::Behind)
					{
						_token = std::move(_tokens[i]);
						_token.segment.end_column = _tokens[i + 1].segment.end_column;
						_token.segment.end_line = _tokens[i + 1].segment.end_line;
						_token.set_keyword(Keyword::NotBehind);
						final_tokens.push_back(std::move(_token));
						++i;
						continue;
					}
				}

				final_tokens.push_back(std::move(_tokens[i]));
			}
			std::swap(_tokens, final_tokens);
		}
	};

	void Lexer::tokenize(const std::string_view script)
	{
		size_t off = 0;
		for (size_t i = 0; i < script.size(); ++i)
		{
			const char c = script[i];
			if (c == '\n')
			{
				_script_lines.push_back(script.substr(off, i - off));
				off = i + 1;
			}
			else if (c == '\r')
			{
				_script_lines.push_back(script.substr(off, i - off));
				if (i + 1 < script.size() && script[i + 1] == '\n')
					++i;
				off = i + 1;
			}
		}
		_script_lines.push_back(script.substr(off, script.size() - off));

		std::vector<Token> tokens;
		Tokenizer tokenizer(script, tokens, _script_lines, _errors);
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

	Token Lexer::start_token() const
	{
		Token token = Token(ScriptSegment(_script_lines));
		token.segment.start_line = 1;
		token.segment.end_line = 1;
		token.segment.start_column = 1;
		token.segment.end_column = 1;
		return token;
	}

	const std::vector<std::string_view>& Lexer::script_lines() const
	{
		return _script_lines;
	}

	const std::vector<LxError>& Lexer::errors() const
	{
		return _errors;
	}
}
