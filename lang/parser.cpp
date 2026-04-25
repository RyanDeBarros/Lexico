#include "parser.h"

#include <stack>
#include <stdexcept>
#include <sstream>

namespace lx
{
	namespace errors
	{
		constexpr const char* UNRECOGNIZED_TOKEN = "unrecognized token";
		constexpr const char* UNRECOGNIZED_DECLARER = "unrecognized declarer (should be 'var' or 'let')";
		constexpr const char* EXPECTED_OPERAND = "expected operand";
		constexpr const char* UNRECOGNIZED_OPERAND = "unrecognized operand";
		constexpr const char* EXPECTED_STATEMENT_END = "unexpected token - expected statement end";
		constexpr const char* EXPECTED_EXPRESSION = "expected expression";
		constexpr const char* EXPECTED_PATTERN_EXPRESSION = "expected pattern expression";
		constexpr const char* EXPECTED_IDENTIFIER = "expected identifier";
		constexpr const char* EXPECTED_SYMBOL = "expected $ symbol";
		constexpr const char* EXPECTED_PREDICATE = "expected predicate";
		constexpr const char* EXPECTED_DATATYPE = "expected data type";
		constexpr const char* EXPECTED_WITH_CLAUSE = "expected 'with' clause";
		constexpr const char* EXPECTED_IN_CLAUSE = "expected 'in' clause";
		constexpr const char* EXPECTED_COLOR_CLAUSE = "expected color symbol";
		constexpr const char* UNRECOGNIZED_COLOR_CLAUSE = "unrecognized color symbol";
		constexpr const char* EXPECTED_ASSIGN = "expected '='";
		constexpr const char* EXPECTED_LPAREN = "expected '('";
		constexpr const char* EXPECTED_RPAREN = "expected ')'";
		constexpr const char* EXPECTED_ARROW = "expected '->'";
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
			ASTBuilder& _builder;
			size_t _local_offset;

		public:
			TokenOffset(ASTBuilder& builder, size_t local_offset)
				: _builder(builder), _local_offset(local_offset)
			{
				_builder._token_offset += _local_offset;
			}

			TokenOffset(const TokenOffset&) = delete;
			TokenOffset(TokenOffset&& other) noexcept = delete;

			~TokenOffset()
			{
				_builder._token_offset -= _local_offset;
			}

			size_t length() const
			{
				return _local_offset;
			}

			void submit()
			{
				_builder.assert_statement_ends();
				_builder.advance(1);

				_builder._token_offset -= _local_offset;
				_local_offset = 0;
			}

			void add(size_t offset)
			{
				_local_offset += offset;
				_builder._token_offset += offset;
			}
		};

		TokenOffset token_offset(size_t offset = 0)
		{
			return TokenOffset(*this, offset);
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
			_stream.advance(n + _token_offset);
		}

		bool eof() const
		{
			return _stream.eof();
		}

		void assert_statement_ends()
		{
			if (!eof() && peek(0).type != TokenType::Newline && peek(0).type != TokenType::EndOfFile)
				throw_error(errors::EXPECTED_STATEMENT_END, 0);
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

		template<typename T>
		T& append_to_context(std::unique_ptr<T>&& node) requires (std::is_base_of_v<ASTNode, T>)
		{
			T* n = node.get();
			context().append(_tree.add(std::move(node)));
			return *n;
		}

		[[noreturn]] void throw_error(const char* cause, size_t peek_offset)
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
			bool parsed =
				parse_pattern_declaration() ||
				parse_delete_pattern() ||
				parse_append_statement() ||
				parse_scope_statement() ||
				parse_find_statement() ||
				parse_filter_statement() ||
				parse_replace_statement() ||
				parse_apply_statement() ||
				parse_page_statement() ||
				parse_function_definition() ||
				parse_variable_declaration() ||
				parse_assignment() ||
				parse_control_statement() ||
				parse_log_statement() ||
				parse_highlight_statement();

			if (!parsed)
				throw_error(errors::UNRECOGNIZED_TOKEN, 0);
		}

		bool parse_pattern_declaration()
		{
			if (eof() || peek(0).type != TokenType::PatternType)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto& identifier = ref(1);
			token_offset(2).submit();
			append_to_context(std::make_unique<PatternDeclaration>(std::move(identifier)));
			return true;
		}

		bool parse_delete_pattern()
		{
			if (eof() || peek(0).type != TokenType::Delete)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto& identifier = ref(1);
			token_offset(2).submit();
			append_to_context(std::make_unique<DeletePattern>(std::move(identifier)));
			return true;
		}

		bool parse_append_statement()
		{
			if (eof() || peek(0).type != TokenType::Append)
				return false;

			if (tokens_left() < 2)
				throw_error(errors::EXPECTED_PATTERN_EXPRESSION, 1);

			auto offset = token_offset(1);
			Expression* expr;
			offset.add(parse_expression(expr));
			offset.submit();
			append_to_context(std::make_unique<AppendStatement>(*expr));
			return true;
		}

		bool parse_scope_statement()
		{
			if (eof() || peek(0).type != TokenType::Scope)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::BuiltinSymbol)
				throw_error(errors::EXPECTED_SYMBOL, 1);

			auto& identifier = ref(1);

			if (tokens_left() < 3)
				throw_error(errors::EXPECTED_EXPRESSION, 2);

			auto offset = token_offset(2);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			append_to_context(std::make_unique<ScopeStatement>(std::move(identifier), *expr));
			return true;
		}

		bool parse_find_statement()
		{
			if (eof() || peek(0).type != TokenType::Find)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto& identifier = ref(1);
			token_offset(2).submit();
			append_to_context(std::make_unique<FindStatement>(std::move(identifier)));
			return true;
		}

		bool parse_filter_statement()
		{
			if (eof() || peek(0).type != TokenType::Filter)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_PREDICATE, 1);

			auto& identifier = ref(1);
			token_offset(2).submit();
			append_to_context(std::make_unique<FilterStatement>(std::move(identifier)));
			return true;
		}

		bool parse_replace_statement()
		{
			if (eof() || peek(0).type != TokenType::Filter)
				return false;

			auto offset = token_offset(1);
			Expression* match_expr;
			offset.add(parse_expression(match_expr));

			if (eof() || peek(0).type != TokenType::With)
				throw_error(errors::EXPECTED_WITH_CLAUSE, 0);

			offset.add(1);
			Expression* string_expr;
			offset.add(parse_expression(string_expr));

			offset.submit();
			append_to_context(std::make_unique<ReplaceStatement>(*match_expr, *string_expr));
			return true;
		}

		bool parse_apply_statement()
		{
			if (eof() || peek(0).type != TokenType::Apply)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto& identifier = ref(1);

			token_offset(2).submit();;
			append_to_context(std::make_unique<ApplyStatement>(std::move(identifier)));
			return true;
		}

		bool parse_page_statement()
		{
			if (eof() || peek(0).type != TokenType::Page)
				return false;

			if (tokens_left() < 2)
				throw_error(errors::EXPECTED_OPERAND, 1);

			if (peek(1).type == TokenType::Push)
			{
				if (tokens_left() < 3)
					throw_error(errors::EXPECTED_EXPRESSION, 2);

				auto offset = token_offset(2);
				Expression* page;
				offset.add(parse_expression(page));

				offset.submit();
				append_to_context(std::make_unique<PagePush>(*page));
				return true;
			}
			else if (peek(1).type == TokenType::Pop)
			{
				token_offset(2).submit();
				append_to_context(std::make_unique<PagePop>());
				return true;
			}
			else if (peek(1).type == TokenType::Delete)
			{
				token_offset(2).submit();
				append_to_context(std::make_unique<PageClearStack>());
				return true;
			}
			else
				throw_error(errors::UNRECOGNIZED_OPERAND, 1);
		}

		bool parse_function_definition()
		{
			if (eof() || peek(0).type != TokenType::Fn)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto& identifier = ref(1);

			if (tokens_left() < 3 || peek(2).type != TokenType::LParen)
				throw_error(errors::EXPECTED_LPAREN, 1);

			auto offset = token_offset(3);

			std::vector<std::pair<Token, Token>> arglist;
			bool comma_ended = false;

			while (!eof() && peek(0).type != TokenType::RParen)
			{
				if (!peek(0).is_datatype())
					throw_error(errors::EXPECTED_DATATYPE, 0);

				if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
					throw_error(errors::EXPECTED_IDENTIFIER, 1);

				arglist.push_back(std::make_pair(std::move(ref(0)), std::move(ref(1))));
				offset.add(2);

				comma_ended = false;
				if (!eof() && peek(0).type != TokenType::Comma)
					break;
				else
					comma_ended = true;
			}

			if (comma_ended)
				throw_error(errors::EXPECTED_DATATYPE, 0);

			if (eof() || peek(0).type != TokenType::RParen)
				throw_error(errors::EXPECTED_RPAREN, 0);

			if (tokens_left() < 2 || peek(1).type != TokenType::Arrow)
				throw_error(errors::EXPECTED_ARROW, 1);

			if (tokens_left() < 3 || !peek(2).is_datatype())
				throw_error(errors::EXPECTED_DATATYPE, 2);

			auto& return_type = ref(2);
			offset.add(3);
			offset.submit();

			enter_context(append_to_context(std::make_unique<FunctionDefinition>(std::move(identifier), std::move(arglist), std::move(return_type))));
			try
			{
				// TODO

				exit_context();
			}
			catch (const SyntaxError&)
			{
				exit_context();
				throw;
			}
		}

		bool parse_variable_declaration()
		{
			if (tokens_left() < 4 || (peek(0).type != TokenType::Var && peek(0).type != TokenType::Let))
				return false;

			bool global = peek(0).type == TokenType::Var;
			if (!global && peek(0).type != TokenType::Let)
				throw_error(errors::UNRECOGNIZED_DECLARER, 0);

			if (peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto& identifier = ref(1);

			if (peek(2).type != TokenType::Assign)
				throw_error(errors::EXPECTED_ASSIGN, 2);

			auto offset = token_offset(3);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			append_to_context(std::make_unique<VariableDeclaration>(global, std::move(identifier), *expr));
			return true;
		}

		bool parse_assignment()
		{
			if (tokens_left() < 3 || peek(0).type != TokenType::Identifier)
				return false;

			if (peek(1).type != TokenType::Assign)
				throw_error(errors::EXPECTED_ASSIGN, 1);

			auto& identifier = ref(0);

			auto offset = token_offset(2);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			append_to_context(std::make_unique<VariableAssignment>(std::move(identifier), *expr));
			return true;
		}

		bool parse_control_statement()
		{
			return parse_break_statement()
				|| parse_continue_statement()
				|| parse_if_statement()
				|| parse_elif_statement()
				|| parse_else_statement()
				|| parse_while_loop()
				|| parse_for_loop();
		}

		bool parse_break_statement()
		{
			if (eof() || peek(0).type != TokenType::Break)
				return false;

			token_offset(1).submit();
			append_to_context(std::make_unique<BreakStatement>());
			return true;
		}

		bool parse_continue_statement()
		{
			if (eof() || peek(0).type != TokenType::Continue)
				return false;

			token_offset(1).submit();
			append_to_context(std::make_unique<ContinueStatement>());
			return true;
		}

		bool parse_if_statement()
		{
			if (eof() || peek(0).type != TokenType::If)
				return false;

			auto offset = token_offset(1);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			enter_context(append_to_context(std::make_unique<IfStatement>(*expr)));
			try
			{
				// TODO

				exit_context();
			}
			catch (const SyntaxError&)
			{
				exit_context();
				throw;
			}

			return true;
		}

		bool parse_elif_statement()
		{
			if (eof() || peek(0).type != TokenType::Elif)
				return false;

			auto offset = token_offset(1);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			enter_context(append_to_context(std::make_unique<ElifStatement>(*expr)));
			try
			{
				// TODO

				exit_context();
			}
			catch (const SyntaxError&)
			{
				exit_context();
				throw;
			}

			return true;
		}

		bool parse_else_statement()
		{
			if (eof() || peek(0).type != TokenType::Else)
				return false;

			token_offset(1).submit();
			enter_context(append_to_context(std::make_unique<ElseStatement>()));
			try
			{
				// TODO

				exit_context();
			}
			catch (const SyntaxError&)
			{
				exit_context();
				throw;
			}

			return true;
		}

		bool parse_while_loop()
		{
			if (eof() || peek(0).type != TokenType::While)
				return false;

			auto offset = token_offset(1);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			enter_context(append_to_context(std::make_unique<WhileLoop>(*expr)));
			try
			{
				// TODO

				exit_context();
			}
			catch (const SyntaxError&)
			{
				exit_context();
				throw;
			}

			return true;
		}

		bool parse_for_loop()
		{
			if (eof() || peek(0).type != TokenType::For)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			if (tokens_left() < 3 || peek(2).type != TokenType::In)
				throw_error(errors::EXPECTED_IN_CLAUSE, 2);

			auto& identifier = ref(1);

			auto offset = token_offset(3);
			Expression* expr;
			offset.add(parse_expression(expr));

			offset.submit();
			enter_context(append_to_context(std::make_unique<ForLoop>(std::move(identifier), *expr)));
			try
			{
				// TODO

				exit_context();
			}
			catch (const SyntaxError&)
			{
				exit_context();
				throw;
			}

			return true;
		}

		bool parse_log_statement()
		{
			if (eof() || peek(0).type != TokenType::Log)
				return false;

			if (tokens_left() < 2)
				throw_error(errors::EXPECTED_IDENTIFIER, 1);

			auto offset = token_offset(1);

			std::vector<Expression*> args;

			while (!eof() && peek(0).type != TokenType::Newline && peek(0).type != TokenType::EndOfFile)
			{
				Expression* expr;
				offset.add(parse_expression(expr));
				args.push_back(expr);

				if (eof() || peek(0).type == TokenType::Newline || peek(0).type == TokenType::EndOfFile)
					break;
				else if (peek(0).type == TokenType::Comma)
					offset.add(1);
				else
					throw_error(errors::EXPECTED_EXPRESSION, 0);
			}

			if (args.empty())
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			offset.submit();
			append_to_context(std::make_unique<LogStatement>(std::move(args)));
			return true;
		}

		bool parse_highlight_statement()
		{
			if (eof() || peek(0).type != TokenType::Highlight)
				return false;

			auto offset = token_offset();
			offset.add(1);

			bool clear = false;
			if (!eof() && peek(0).type == TokenType::Delete)
			{
				offset.add(1);
				clear = true;
			}

			BuiltinSymbol color = BuiltinSymbol::Yellow;
			Expression* expr = nullptr;

			if (!eof() && peek(0).type != TokenType::Color)
				offset.add(parse_expression(expr));

			if (!eof() && peek(0).type == TokenType::Color)
			{
				offset.add(1);
				if (eof() || peek(0).type != TokenType::BuiltinSymbol)
					throw_error(errors::EXPECTED_COLOR_CLAUSE, 0);

				if (auto c = parse_builtin_symbol(peek(0).lexeme))
				{
					color = *c;
					offset.add(1);
				}
				else
					throw_error(errors::UNRECOGNIZED_COLOR_CLAUSE, 0);
			}

			offset.submit();
			append_to_context(std::make_unique<HighlightStatement>(clear, expr, color));
			return true;
		}

		size_t parse_expression(Expression*& expr)
		{
			expr = nullptr;

			// TODO
			throw_error(errors::UNRECOGNIZED_TOKEN, 0);
			return 0;
		}

		size_t parse_pattern_expression(PatternExpression*& expr)
		{
			expr = nullptr;

			// TODO
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
