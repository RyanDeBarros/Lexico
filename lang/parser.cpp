#include "parser.h"

#include <stack>
#include <stdexcept>
#include <sstream>

namespace lx
{
	namespace errors
	{
		constexpr const char* UNRECOGNIZED_TOKEN = "unrecognized token";
		constexpr const char* EXPECTED_OPERAND = "expected operand";
		constexpr const char* UNRECOGNIZED_OPERAND = "unrecognized operand";
		constexpr const char* EXPECTED_STATEMENT_END = "unexpected token - expected statement end";
		constexpr const char* EXPECTED_EXPRESSION = "expected expression";
		constexpr const char* EXPECTED_IDENTIFIER = "expected identifier";
		constexpr const char* EXPECTED_PREDICATE = "expected predicate";
		constexpr const char* EXPECTED_WITH_CLAUSE = "expected 'with' clause";
		constexpr const char* EXPECTED_COLOR_CLAUSE = "expected color symbol";
		constexpr const char* UNRECOGNIZED_COLOR_CLAUSE = "unrecognized color symbol";
		constexpr const char* EXPECTED_ASSIGN_OPERATOR = "expected '='";
	};

	class ASTBuilder
	{
		TokenStream& _stream;
		std::vector<SyntaxError>& _errors;
		const std::vector<std::string_view>& _script_lines;
		size_t _token_offset = 0;
		AbstractSyntaxTree& _tree;
		std::stack<Block*> _context_stack;

	public:
		ASTBuilder(TokenStream& stream, std::vector<SyntaxError>& errors, const std::vector<std::string_view>& script_lines, AbstractSyntaxTree& tree)
			: _stream(stream), _errors(errors), _script_lines(script_lines), _tree(tree)
		{
			stream.seek();
			size_t tokens_left = SIZE_MAX;
			while (!_stream.eof() && _stream.tokens_left() < tokens_left)
			{
				try
				{
					parse_statement();
				}
				catch (const SyntaxError& e)
				{
					_errors.push_back(e);
					while (!_stream.eof() && _stream.peek(0).type != TokenType::Newline && _stream.peek(0).type != TokenType::EndOfFile)
						_stream.advance();
					if (!_stream.eof())
						_stream.advance();
				}

				tokens_left = _stream.tokens_left();
			}
		}

	private:
		class TokenOffset
		{
			size_t& _full_offset;
			size_t _local_offset;
			bool alive = true;

		public:
			TokenOffset(size_t& full_offset, size_t local_offset)
				: _full_offset(full_offset), _local_offset(local_offset)
			{
				_full_offset += _local_offset;
			}

			TokenOffset(const TokenOffset&) = delete;

			TokenOffset(TokenOffset&& other) noexcept
				: alive(other.alive), _full_offset(other._full_offset), _local_offset(other._local_offset)
			{
				other.alive = false;
			}

			~TokenOffset()
			{
				if (alive)
					_full_offset -= _local_offset;
			}

			size_t offset() const
			{
				return _local_offset;
			}
			
			static size_t offset(const std::vector<TokenOffset>& offsets)
			{
				size_t off = 0;
				for (const TokenOffset& offset : offsets)
					off += offset.offset();
				return off;
			}
		};

		TokenOffset token_offset(size_t offset)
		{
			return TokenOffset(_token_offset, offset);
		}

		size_t tokens_left()
		{
			size_t actual = _stream.tokens_left();
			return actual > _token_offset ? actual - _token_offset : 0;
		}

		const Token& peek(size_t n)
		{
			return _stream.peek(n + _token_offset);
		}

		Token& ref(size_t n)
		{
			return _stream.ref(n + _token_offset);
		}

		void advance(size_t n)
		{
			_stream.advance(n);
		}

		bool eof() const
		{
			return _stream.eof();
		}

		void assert_statement_ends(size_t n)
		{
			if (tokens_left() > n && peek(n).type != TokenType::Newline && peek(n).type != TokenType::EndOfFile)
				throw_error(errors::EXPECTED_STATEMENT_END, n);
		}

		Block& context()
		{
			return _context_stack.empty() ? _tree.root() : *_context_stack.top();
		}

		void enter_context(Block& block)
		{
			_context_stack.push(&block);
		}

		void exit_context()
		{
			if (_context_stack.empty())
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": context stack is empty";
				throw std::runtime_error(ss.str());
			}

			_context_stack.pop();
		}

		void append_to_context(std::unique_ptr<ASTNode>&& node)
		{
			context().append(_tree.add(std::move(node)));
		}

		void throw_error(const char* cause, size_t peek_offset)
		{
			std::stringstream ss;
			ss << "[Parser Error] " << cause;
			if (!eof())
			{
				ss << ":\n\t";
				if (peek_offset >= tokens_left())
				{
					ss << _script_lines.back();
					ss << '\n';
					Token token = peek(tokens_left() - 1);
					token.start_column = token.end_column;
					++token.end_column;
					token.start_line = token.end_line;
					ss << SyntaxError::underline(token);
				}
				else
				{
					ss << _script_lines[peek(peek_offset).start_line];
					ss << '\n';
					ss << SyntaxError::underline(peek(peek_offset));
				}
			}
			throw SyntaxError(ss.str());
		}

		void parse_statement()
		{
			if (parse_pattern_declaration())
				return;
			if (parse_delete_pattern())
				return;
			if (parse_append_statement())
				return;
			if (parse_scope_statement())
				return;
			if (parse_find_statement())
				return;
			if (parse_filter_statement())
				return;
			if (parse_replace_statement())
				return;
			if (parse_apply_statement())
				return;
			if (parse_page_statement())
				return;
			if (parse_function_definition())
				return;
			if (parse_variable_declaration())
				return;
			if (parse_assignment())
				return;
			if (parse_control_statement())
				return;
			if (parse_log_statement())
				return;
			if (parse_highlight_statement())
				return;

			throw_error(errors::UNRECOGNIZED_TOKEN, 0);
		}

		bool parse_pattern_declaration()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::PatternType)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			assert_statement_ends(2);
			append_to_context(std::make_unique<PatternDeclaration>(std::move(ref(1))));
			advance(3);
			return true;
		}

		bool parse_delete_pattern()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Delete)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			assert_statement_ends(2);
			append_to_context(std::make_unique<DeletePattern>(std::move(ref(1))));
			advance(3);
			return true;
		}

		bool parse_append_statement()
		{
			// TODO
			return false;
		}

		bool parse_scope_statement()
		{
			// TODO
			return false;
		}

		bool parse_find_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Find)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			assert_statement_ends(2);
			append_to_context(std::make_unique<FindStatement>(std::move(ref(1))));
			advance(3);
			return true;
		}

		bool parse_filter_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Filter)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_PREDICATE, 1);

			assert_statement_ends(2);
			append_to_context(std::make_unique<FilterStatement>(std::move(ref(1))));
			advance(3);
			return true;
		}

		bool parse_replace_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Filter)
				return false;

			auto match_offset = token_offset(1);
			Expression* match_expr;
			size_t match_length = parse_expression(match_expr);

			auto with_offset = token_offset(match_length);

			if (eof() || peek(0).type != TokenType::With)
				throw_error(errors::EXPECTED_WITH_CLAUSE, 0);

			auto string_offset = token_offset(1);
			Expression* string_expr;
			size_t string_length = parse_expression(string_expr);

			assert_statement_ends(string_length);
			append_to_context(std::make_unique<ReplaceStatement>(*match_expr, *string_expr));
			advance(match_offset.offset() + with_offset.offset() + string_offset.offset() + string_length + 1);
			return true;
		}

		bool parse_apply_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Apply)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			assert_statement_ends(2);
			append_to_context(std::make_unique<ApplyStatement>(std::move(ref(1))));
			advance(3);
			return true;
		}

		bool parse_page_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Page)
				return false;

			if (tokens_left() < 2)
				throw_error(errors::EXPECTED_OPERAND, 1);

			if (peek(1).type == TokenType::Push)
			{
				if (tokens_left() < 3)
					throw_error(errors::EXPECTED_EXPRESSION, 2);

				auto offset = token_offset(2);
				Expression* page;
				size_t length = parse_expression(page);

				assert_statement_ends(length);
				append_to_context(std::make_unique<PagePush>(*page));
				advance(offset.offset() + length + 1);
				return true;
			}
			else if (peek(1).type == TokenType::Pop)
			{
				assert_statement_ends(2);
				append_to_context(std::make_unique<PagePop>());
				advance(3);
				return true;
			}
			else if (peek(1).type == TokenType::Delete)
			{
				assert_statement_ends(2);
				append_to_context(std::make_unique<PageClearStack>());
				advance(3);
				return true;
			}
			else
				throw_error(errors::UNRECOGNIZED_OPERAND, 1);
		}

		bool parse_function_definition()
		{
			// TODO
			return false;
		}

		bool parse_variable_declaration()
		{
			if (tokens_left() < 4)
				return false;

			if (peek(0).type != TokenType::Var && peek(0).type != TokenType::Let)
				return false;

			if (peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			if (peek(2).type != TokenType::Assign)
				throw_error(errors::EXPECTED_ASSIGN_OPERATOR, 2);

			auto offset = token_offset(3);
			Expression* expr;
			size_t length = parse_expression(expr);

			bool global = peek(0).type == TokenType::Var;

			assert_statement_ends(length);
			append_to_context(std::make_unique<VariableDeclaration>(global, std::move(ref(1)), *expr));
			advance(offset.offset() + length + 1);
			return true;
		}

		bool parse_assignment()
		{
			if (tokens_left() < 3)
				return false;

			if (peek(0).type != TokenType::Identifier)
				return false;

			if (peek(1).type != TokenType::Assign)
				throw_error(errors::EXPECTED_ASSIGN_OPERATOR, 1);

			auto offset = token_offset(2);
			Expression* expr;
			size_t length = parse_expression(expr);

			assert_statement_ends(length);
			append_to_context(std::make_unique<VariableAssignment>(std::move(ref(0)), *expr));
			advance(offset.offset() + length + 1);
			return true;
		}

		bool parse_control_statement()
		{
			// TODO
			return false;
		}

		bool parse_log_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Log)
				return false;

			if (tokens_left() < 2)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto offset = token_offset(1);

			std::vector<Expression*> args;
			std::vector<TokenOffset> offsets;

			while (!eof() && peek(0).type != TokenType::Newline && peek(0).type != TokenType::EndOfFile)
			{
				Expression* expr;
				size_t length = parse_expression(expr);
				args.push_back(expr);
				offsets.push_back(token_offset(length));

				if (eof() || peek(0).type == TokenType::Newline || peek(0).type == TokenType::EndOfFile)
					break;
				else if (peek(0).type == TokenType::Comma)
					offsets.push_back(token_offset(1));
				else
					throw_error(errors::EXPECTED_EXPRESSION, 0);
			}

			if (args.empty())
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			assert_statement_ends(0);
			append_to_context(std::make_unique<LogStatement>(std::move(args)));
			advance(offset.offset() + TokenOffset::offset(offsets) + 1);
			return true;
		}

		bool parse_highlight_statement()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Highlight)
				return false;

			std::vector<TokenOffset> offsets;
			offsets.push_back(token_offset(1));

			bool clear = false;
			if (!eof() && peek(0).type == TokenType::Delete)
			{
				offsets.push_back(token_offset(1));
				clear = true;
			}

			BuiltinSymbol color = BuiltinSymbol::Yellow;
			Expression* expr = nullptr;

			if (!eof() && peek(0).type != TokenType::Color)
			{
				size_t length = parse_expression(expr);
				offsets.push_back(token_offset(length));
			}

			if (!eof() && peek(0).type == TokenType::Color)
			{
				offsets.push_back(token_offset(1));
				if (eof() || peek(0).type != TokenType::BuiltinSymbol)
					throw_error(errors::EXPECTED_COLOR_CLAUSE, 0);

				if (auto c = parse_builtin_symbol(peek(0).lexeme))
				{
					color = *c;
					offsets.push_back(token_offset(1));
				}
				else
					throw_error(errors::UNRECOGNIZED_COLOR_CLAUSE, 0);
			}

			assert_statement_ends(0);
			append_to_context(std::make_unique<HighlightStatement>(clear, expr, color));
			advance(TokenOffset::offset(offsets) + 1);
			return true;
		}

		size_t parse_expression(Expression*& expr)
		{
			expr = nullptr;

			// TODO Expressions should copy tokens for now, since they might not be published. Use string_view in tokens instead, but that means including \ from escape characters in string. So, string literals should have a resolve method that will produce a new string without the \s.

			throw_error(errors::UNRECOGNIZED_TOKEN, 0);
			return 0;
		}
	};

	void Parser::parse(TokenStream& stream, const std::vector<std::string_view>& script_lines)
	{
		ASTBuilder builder(stream, _errors, script_lines, _tree);
	}

	const AbstractSyntaxTree& Parser::tree() const
	{
		return _tree;
	}

	const std::vector<SyntaxError>& Parser::errors() const
	{
		return _errors;
	}
}
