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
		constexpr const char* EXPECTED_IF_END = "expected 'end if' statement";
		constexpr const char* EXPECTED_IF = "expected 'if'";
		constexpr const char* EXPECTED_FN_END = "expected 'end fn' statement";
		constexpr const char* EXPECTED_FN = "expected 'fn'";
		constexpr const char* EXPECTED_WHILE_END = "expected 'end while' statement";
		constexpr const char* EXPECTED_WHILE = "expected 'while'";
		constexpr const char* EXPECTED_FOR_END = "expected 'end for' statement";
		constexpr const char* EXPECTED_FOR = "expected 'for'";
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
			while (!eof())
			{
				try
				{
					parse_statement();
				}
				catch (const SyntaxError& e)
				{
					catch_error(e);
				}
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

		size_t tokens_left() const
		{
			size_t actual = _stream.tokens_left();
			return actual > _token_offset ? actual - _token_offset : 0;
		}

		const Token& peek(size_t n) const
		{
			return _stream.peek(n + _token_offset);
		}

		void advance(size_t n)
		{
			_stream.advance(n + _token_offset);
		}

		bool eof() const
		{
			return _stream.eof();
		}

		void assert_statement_ends() const
		{
			if (continue_statement())
				throw_error(errors::EXPECTED_STATEMENT_END, 0);
		}

		bool continue_statement() const
		{
			return tokens_exist(0) && peek_token_is_not(0, TokenType::Newline) && peek_token_is_not(0, TokenType::EndOfFile);
		}

		Block& context()
		{
			return _context_stack.empty() ? _tree.root() : *_context_stack.top();
		}

		class Context
		{
			ASTBuilder& _builder;

		public:
			Context(ASTBuilder& builder, Block& block)
				: _builder(builder)
			{
				_builder._context_stack.push(&block);
			}

			Context(const Context&) = delete;
			Context(Context&&) = delete;

			~Context()
			{
				_builder._context_stack.pop();
			}
		};

		Context context(Block& block)
		{
			return Context(*this, block);
		}

		template<typename T>
		T& append_to_context(std::unique_ptr<T>&& node) requires (std::is_base_of_v<ASTNode, T>)
		{
			T* n = node.get();
			context().append(_tree.add(std::move(node)));
			return *n;
		}

		[[noreturn]] void throw_error(const char* cause, size_t peek_offset) const
		{
			std::stringstream ss;
			ss << "[Parser Error] " << cause;
			if (!eof())
			{
				ss << ":\n";
				// TODO line numbers are off by one
				if (peek_offset >= tokens_left() || peek(peek_offset).start_line >= _script_lines.size())
				{
					Token token = peek(tokens_left() - 1);
					token.start_column = token.end_column;
					++token.end_column;
					token.start_line = token.end_line;
					
					std::string line_number = token.line_number_prefix();
					unsigned int tabs = 1 + line_number.size() / 4;
					ss << "    " << line_number << _script_lines.back() << '\n' << SyntaxError::underline(token, tabs);
				}
				else
				{
					const Token& token = peek(peek_offset);
					std::string line_number = token.line_number_prefix();
					unsigned int tabs = 1 + line_number.size() / 4;
					ss << "    " << line_number << _script_lines[token.start_line - 1] << '\n' << SyntaxError::underline(token, tabs);
				}
			}
			throw SyntaxError(ss.str());
		}

		void catch_error(const SyntaxError& e)
		{
			_errors.push_back(e);
			while (continue_statement())
				advance(1);
			if (!eof())
				advance(1);
		}

		Token& parse_token(size_t n, TokenType type, const char* err)
		{
			if (tokens_exist(n) && peek(n).type == type)
				return _stream.ref(n + _token_offset);
			else
				throw_error(err, n);
		}

		Token& parse_datatype(size_t n)
		{
			if (tokens_exist(n) && peek(n).is_datatype())
				return _stream.ref(n + _token_offset);
			else
				throw_error(errors::EXPECTED_DATATYPE, n);
		}

		bool tokens_exist(size_t n) const
		{
			return n < tokens_left();
		}

		bool peek_token_type(size_t n, TokenType type) const
		{
			return tokens_exist(n) && peek(n).type == type;
		}

		bool peek_token_is_not(size_t n, TokenType type) const
		{
			return tokens_exist(n) && peek(n).type != type;
		}

		void assert_tokens_exist(size_t n, const char* err) const
		{
			if (!tokens_exist(n))
				throw_error(err, n);
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

		template<typename Node>
		bool parse_declaration(TokenType first)
		{
			if (!peek_token_type(0, first))
				return false;

			auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);

			token_offset(2).submit();
			append_to_context(std::make_unique<Node>(std::move(identifier)));
			return true;
		}

		bool parse_pattern_declaration()
		{
			return parse_declaration<PatternDeclaration>(TokenType::PatternType);
		}

		bool parse_delete_pattern()
		{
			return parse_declaration<DeletePattern>(TokenType::Delete);
		}

		bool parse_append_statement()
		{
			if (!peek_token_type(0, TokenType::Append))
				return false;

			assert_tokens_exist(1, errors::EXPECTED_PATTERN_EXPRESSION);
			auto offset = token_offset(1);
			PatternExpression& expr = parse_pattern_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<AppendStatement>(expr));
			return true;
		}

		bool parse_scope_statement()
		{
			if (!peek_token_type(0, TokenType::Scope))
				return false;
			
			auto& identifier = parse_token(1, TokenType::BuiltinSymbol, errors::EXPECTED_SYMBOL);
			assert_tokens_exist(2, errors::EXPECTED_EXPRESSION);
			auto offset = token_offset(2);
			Expression& expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<ScopeStatement>(std::move(identifier), expr));
			return true;
		}

		bool parse_find_statement()
		{
			return parse_declaration<FindStatement>(TokenType::Find);
		}

		bool parse_filter_statement()
		{
			return parse_declaration<FilterStatement>(TokenType::Filter);
		}

		bool parse_replace_statement()
		{
			if (!peek_token_type(0, TokenType::Filter))
				return false;

			auto offset = token_offset(1);
			Expression& match_expr = parse_expression(offset);
			parse_token(0, TokenType::With, errors::EXPECTED_WITH_CLAUSE);
			offset.add(1);
			Expression& string_expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<ReplaceStatement>(match_expr, string_expr));
			return true;
		}

		bool parse_apply_statement()
		{
			return parse_declaration<ApplyStatement>(TokenType::Apply);
		}

		bool parse_page_statement()
		{
			if (!peek_token_type(0, TokenType::Page))
				return false;

			assert_tokens_exist(1, errors::EXPECTED_OPERAND);

			if (peek_token_type(1, TokenType::Push))
			{
				assert_tokens_exist(2, errors::EXPECTED_EXPRESSION);
				auto offset = token_offset(2);
				Expression& page = parse_expression(offset);
				offset.submit();
				append_to_context(std::make_unique<PagePush>(page));
				return true;
			}
			else if (peek_token_type(1, TokenType::Pop))
			{
				token_offset(2).submit();
				append_to_context(std::make_unique<PagePop>());
				return true;
			}
			else if (peek_token_type(1, TokenType::Delete))
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
			if (!peek_token_type(0, TokenType::Fn))
				return false;

			auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
			parse_token(2, TokenType::LParen, errors::EXPECTED_LPAREN);
			auto offset = token_offset(3);

			std::vector<std::pair<Token, Token>> arglist;
			bool comma_ended = false;

			while (peek_token_is_not(0, TokenType::RParen))
			{
				auto& datatype = parse_datatype(0);
				auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
				arglist.push_back(std::make_pair(std::move(datatype), std::move(identifier)));
				offset.add(2);

				comma_ended = false;
				if (peek_token_is_not(0, TokenType::Comma))
					break;
				else
					comma_ended = true;
			}

			if (comma_ended)
				throw_error(errors::EXPECTED_DATATYPE, 0);

			parse_token(0, TokenType::RParen, errors::EXPECTED_RPAREN);
			parse_token(1, TokenType::Arrow, errors::EXPECTED_ARROW);
			auto& return_type = parse_datatype(2);
			offset.add(3);

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<FunctionDefinition>(std::move(identifier), std::move(arglist), std::move(return_type))));
			parse_simple_block(TokenType::Fn, errors::EXPECTED_FN_END, errors::EXPECTED_FN);
			return true;
		}

		bool parse_variable_declaration()
		{
			bool global;
			if (peek_token_type(0, TokenType::Var))
				global = true;
			else if (peek_token_type(0, TokenType::Let))
				global = false;
			else
				return false;

			auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
			parse_token(2, TokenType::Assign, errors::EXPECTED_ASSIGN);

			auto offset = token_offset(3);
			Expression& expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<VariableDeclaration>(global, std::move(identifier), expr));
			return true;
		}

		bool parse_assignment()
		{
			if (!peek_token_type(0, TokenType::Identifier))
				return false;

			auto& identifier = parse_token(0, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
			parse_token(1, TokenType::Assign, errors::EXPECTED_ASSIGN);

			auto offset = token_offset(2);
			Expression& expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<VariableAssignment>(std::move(identifier), expr));
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

		template<typename Node>
		bool parse_unary_control_statement(TokenType type)
		{
			if (!peek_token_type(0, type))
				return false;

			token_offset(1).submit();
			append_to_context(std::make_unique<Node>());
			return true;
		}

		bool parse_break_statement()
		{
			return parse_unary_control_statement<BreakStatement>(TokenType::Break);
		}

		bool parse_continue_statement()
		{
			return parse_unary_control_statement<ContinueStatement>(TokenType::Continue);
		}

		template<typename Node>
		bool parse_if_control_block(TokenType type)
		{
			if (!peek_token_type(0, type))
				return false;

			auto offset = token_offset(1);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<Node>(expr)));
			parse_if_block();
			return true;
		}

		bool parse_if_statement()
		{
			return parse_if_control_block<IfStatement>(TokenType::If);
		}

		bool parse_elif_statement()
		{
			return parse_if_control_block<ElifStatement>(TokenType::Elif);
		}

		bool parse_else_statement()
		{
			if (!peek_token_type(0, TokenType::Else))
				return false;

			token_offset(1).submit();
			auto ctx = context(append_to_context(std::make_unique<ElseStatement>()));
			parse_simple_block(TokenType::If, errors::EXPECTED_IF_END, errors::EXPECTED_IF);
			return true;
		}

		bool parse_while_loop()
		{
			if (!peek_token_type(0, TokenType::While))
				return false;

			auto offset = token_offset(1);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<WhileLoop>(expr)));
			parse_simple_block(TokenType::While, errors::EXPECTED_WHILE_END, errors::EXPECTED_WHILE);
			return true;
		}

		bool parse_for_loop()
		{
			if (!peek_token_type(0, TokenType::For))
				return false;

			auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
			parse_token(2, TokenType::In, errors::EXPECTED_IN_CLAUSE);

			auto offset = token_offset(3);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<ForLoop>(std::move(identifier), expr)));
			parse_simple_block(TokenType::For, errors::EXPECTED_FOR_END, errors::EXPECTED_FOR);
			return true;
		}

		bool parse_log_statement()
		{
			if (!peek_token_type(0, TokenType::Log))
				return false;

			auto offset = token_offset(1);

			std::vector<Expression*> args;
			bool comma_ended = false;

			while (continue_statement())
			{
				args.push_back(&parse_expression(offset));
				comma_ended = false;
				if (peek_token_type(0, TokenType::Comma))
				{
					offset.add(1);
					comma_ended = true;
				}
			}

			if (comma_ended || args.empty())
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			offset.submit();
			append_to_context(std::make_unique<LogStatement>(std::move(args)));
			return true;
		}

		bool parse_highlight_statement()
		{
			if (!peek_token_type(0, TokenType::Highlight))
				return false;

			auto offset = token_offset();
			offset.add(1);

			bool clear = false;
			if (peek_token_type(0, TokenType::Delete))
			{
				offset.add(1);
				clear = true;
			}

			BuiltinSymbol color = BuiltinSymbol::Yellow;
			Expression* expr = nullptr;

			if (peek_token_is_not(0, TokenType::Color))
				expr = &parse_expression(offset);

			if (peek_token_type(0, TokenType::Color))
			{
				offset.add(1);

				if (auto c = parse_builtin_symbol(parse_token(0, TokenType::BuiltinSymbol, errors::EXPECTED_COLOR_CLAUSE).lexeme))
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

		bool parse_end_statement(TokenType end, const char* missing_end_kw_err)
		{
			if (!peek_token_type(0, TokenType::End))
				return false;

			parse_token(1, end, missing_end_kw_err);
			token_offset(2).submit();
			return true;
		}

		void parse_simple_block(TokenType end, const char* missing_end_err, const char* missing_end_kw_err)
		{
			while (!eof())
			{
				if (parse_end_statement(end, missing_end_kw_err))
					return;

				parse_statement();
			}

			throw_error(missing_end_err, 0);
		}

		void parse_if_block()
		{
			while (!eof())
			{
				if (parse_end_statement(TokenType::If, errors::EXPECTED_IF) || peek_token_type(0, TokenType::Elif) || peek_token_type(0, TokenType::Else))
					return;

				parse_statement();
			}

			throw_error(errors::EXPECTED_IF_END, 0);
		}

		Expression& parse_expression(TokenOffset& offset)
		{
			// TODO remember to update offset
			throw_error(errors::UNRECOGNIZED_TOKEN, 0);
		}

		PatternExpression& parse_pattern_expression(TokenOffset& offset)
		{
			// TODO remember to update offset
			throw_error(errors::UNRECOGNIZED_TOKEN, 0);
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
