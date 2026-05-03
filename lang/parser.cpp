#include "parser.h"

#include <stack>
#include <stdexcept>
#include <sstream>

namespace lx
{
	namespace errors
	{
		constexpr const char* UNRECOGNIZED_TOKEN = "unrecognized token";
		constexpr const char* UNRECOGNIZED_SYMBOL = "unrecognized symbol";
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
		constexpr const char* EXPECTED_LBRACKET = "expected '['";
		constexpr const char* EXPECTED_RBRACKET = "expected ']'";
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
		std::vector<LxError>& _errors;
		size_t _token_offset = 0;
		AbstractSyntaxTree& _tree;
		std::stack<Block*> _context_stack;

	public:
		ASTBuilder(TokenStream& stream, std::vector<LxError>& errors, AbstractSyntaxTree& tree)
			: _stream(stream), _errors(errors), _tree(tree)
		{
			while (!eof())
			{
				try
				{
					parse_statement();
				}
				catch (const LxError& e)
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

		const Token& peek(int n) const
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

		bool continue_statement(size_t n = 0) const
		{
			return tokens_exist(n) && peek_token_is_not(n, TokenType::Newline) && peek_token_is_not(n, TokenType::EndOfFile);
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
			if (eof())
				throw LxError(ErrorType::Syntax, cause);
			else
			{
				if (peek_offset < tokens_left())
					throw LxError::segment_error(peek(peek_offset).segment, ErrorType::Syntax, cause);
				else
				{
					ScriptSegment segment = peek(tokens_left() - 1).segment;
					++segment.start_column;
					throw LxError::segment_error(segment, ErrorType::Syntax, cause);
				}
			}
		}

		void catch_error(const LxError& e)
		{
			_errors.push_back(e);
			while (continue_statement())
				advance(1);
			if (!eof())
				advance(1);
		}

		Token& ref(int n)
		{
			return _stream.ref(n + _token_offset);
		}

		Token& parse_token(int n, TokenType type, const char* err)
		{
			if (tokens_exist(n) && peek(n).type == type)
				return ref(n);
			else
				throw_error(err, n);
		}

		Token& parse_datatype(int n)
		{
			if (tokens_exist(n) && peek(n).is_datatype())
				return ref(n);
			else
				throw_error(errors::EXPECTED_DATATYPE, n);
		}

		bool tokens_exist(size_t n) const
		{
			return n < tokens_left();
		}

		bool peek_token_is(size_t n, TokenType type) const
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
				parse_function_call() ||
				parse_control_statement() ||
				parse_log_statement() ||
				parse_highlight_statement() ||
				parse_direct_expression_statement();

			if (!parsed)
				throw_error(errors::UNRECOGNIZED_TOKEN, 0);
		}

		template<typename Node>
		bool parse_declaration(TokenType first)
		{
			if (!peek_token_is(0, first))
				return false;

			auto& kw_token = ref(0);
			auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);

			token_offset(2).submit();
			append_to_context(std::make_unique<Node>(std::move(kw_token), std::move(identifier)));
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
			if (!peek_token_is(0, TokenType::Append))
				return false;

			assert_tokens_exist(1, errors::EXPECTED_PATTERN_EXPRESSION);
			auto& append_token = ref(0);
			auto offset = token_offset(1);
			Expression& expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<AppendStatement>(std::move(append_token), expr));
			return true;
		}

		bool parse_scope_statement()
		{
			if (!peek_token_is(0, TokenType::Scope))
				return false;
			
			auto& scope_token = ref(0);
			auto& symbol_token = parse_token(1, TokenType::BuiltinSymbol, errors::EXPECTED_SYMBOL);
			const auto specifier = parse_builtin_symbol(symbol_token.lexeme);
			if (!specifier)
				throw_error(errors::EXPECTED_SYMBOL, 1);

			assert_tokens_exist(2, errors::EXPECTED_EXPRESSION);
			auto offset = token_offset(2);
			Expression& expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<ScopeStatement>(std::move(scope_token), std::move(symbol_token), *specifier, expr));
			return true;
		}

		bool parse_find_statement()
		{
			if (!peek_token_is(0, TokenType::Find))
				return false;

			auto& find_token = ref(0);
			auto offset = token_offset(1);
			Expression& pattern = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<FindStatement>(std::move(find_token), pattern));
			return true;
		}

		bool parse_filter_statement()
		{
			return parse_declaration<FilterStatement>(TokenType::Filter);
		}

		bool parse_replace_statement()
		{
			if (!peek_token_is(0, TokenType::Filter))
				return false;

			auto& replace_token = ref(0);
			auto offset = token_offset(1);
			Expression& match_expr = parse_expression(offset);
			parse_token(0, TokenType::With, errors::EXPECTED_WITH_CLAUSE);
			offset.add(1);
			Expression& string_expr = parse_expression(offset);
			offset.submit();
			append_to_context(std::make_unique<ReplaceStatement>(std::move(replace_token), match_expr, string_expr));
			return true;
		}

		bool parse_apply_statement()
		{
			return parse_declaration<ApplyStatement>(TokenType::Apply);
		}

		bool parse_page_statement()
		{
			if (!peek_token_is(0, TokenType::Page))
				return false;

			assert_tokens_exist(1, errors::EXPECTED_OPERAND);

			if (peek_token_is(1, TokenType::Push))
			{
				assert_tokens_exist(2, errors::EXPECTED_EXPRESSION);
				auto& page_token = ref(0);
				auto offset = token_offset(2);
				Expression& page = parse_expression(offset);
				offset.submit();
				append_to_context(std::make_unique<PagePush>(std::move(page_token), page));
				return true;
			}
			else if (peek_token_is(1, TokenType::Pop))
			{
				auto& page_token = ref(0);
				token_offset(2).submit();
				append_to_context(std::make_unique<PagePop>(std::move(page_token)));
				return true;
			}
			else if (peek_token_is(1, TokenType::Delete))
			{
				auto& page_token = ref(0);
				token_offset(2).submit();
				append_to_context(std::make_unique<PageClearStack>(std::move(page_token)));
				return true;
			}
			else
				throw_error(errors::UNRECOGNIZED_OPERAND, 1);
		}

		bool parse_function_definition()
		{
			if (!peek_token_is(0, TokenType::Fn))
				return false;

			auto& fn_token = ref(0);
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
				{
					comma_ended = true;
					offset.add(1);
				}
			}

			if (comma_ended)
				throw_error(errors::EXPECTED_DATATYPE, 0);

			parse_token(0, TokenType::RParen, errors::EXPECTED_RPAREN);
			offset.add(1);

			Token* return_type = nullptr;
			if (peek_token_is(0, TokenType::Arrow))
			{
				parse_token(0, TokenType::Arrow, errors::EXPECTED_ARROW);
				return_type = &parse_datatype(1);
				offset.add(2);
			}

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<FunctionDefinition>(std::move(fn_token), std::move(identifier), std::move(arglist),
				return_type ? std::make_optional<Token>(std::move(*return_type)) : std::nullopt)));
			parse_simple_block(TokenType::Fn, errors::EXPECTED_FN_END, errors::EXPECTED_FN);
			return true;
		}

		bool parse_variable_declaration()
		{
			bool global;
			if (peek_token_is(0, TokenType::Var))
				global = true;
			else if (peek_token_is(0, TokenType::Let))
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
			if (!(peek_token_is(0, TokenType::Identifier) || peek_token_is(0, TokenType::Percent)) || !peek_token_is(1, TokenType::Assign))
				return false;

			auto& identifier = ref(0);
			parse_token(1, TokenType::Assign, errors::EXPECTED_ASSIGN);

			auto offset = token_offset(2);
			Expression& expr = parse_expression(offset);
			offset.submit();

			if (identifier.type == TokenType::Percent)
				append_to_context(std::make_unique<GlobalMatchesAssignment>(std::move(identifier), expr));
			else
				append_to_context(std::make_unique<VariableAssignment>(std::move(identifier), expr));
			return true;
		}

		bool parse_function_call()
		{
			if (!peek_token_is(0, TokenType::Identifier) || !peek_token_is(1, TokenType::LParen))
				return false;

			auto& identifier = ref(0);
			auto offset = token_offset(1);
			auto& expr = parse_function_call_expression(std::move(identifier), offset);
			offset.submit();
			context().append(expr);
			return true;
		}

		bool parse_direct_expression_statement()
		{
			auto offset = token_offset();
			Expression& expr = parse_expression(offset);
			if (!expr.imperative())
				throw LxError::segment_error(expr.segment(), ErrorType::Syntax, "expression is not imperative");

			offset.submit();
			context().append(expr);
			return true;
		}

		bool parse_control_statement()
		{
			return parse_break_statement()
				|| parse_continue_statement()
				|| parse_return_statement()
				|| parse_if_statement()
				|| parse_while_loop()
				|| parse_for_loop();
		}

		template<typename Node>
		bool parse_unary_control_statement(TokenType type)
		{
			if (!peek_token_is(0, type))
				return false;

			auto& token = ref(0);
			token_offset(1).submit();
			append_to_context(std::make_unique<Node>(std::move(token)));
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

		bool parse_return_statement()
		{
			if (!peek_token_is(0, TokenType::Return))
				return false;
			
			auto& return_token = ref(0);
			auto offset = token_offset(1);

			Expression* expr = nullptr;
			if (continue_statement())
				expr = &parse_expression(offset);

			offset.submit();
			append_to_context(std::make_unique<ReturnStatement>(std::move(return_token), expr));
			return true;
		}

		bool parse_if_statement()
		{
			if (!peek_token_is(0, TokenType::If))
				return false;

			auto& if_token = ref(0);
			auto offset = token_offset(1);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto& stmt = append_to_context(std::make_unique<IfStatement>(std::move(if_token), expr));
			auto ctx = context(stmt);
			parse_if_block(stmt);
			return true;
		}

		bool parse_elif_statement(IfConditional& cond)
		{
			if (!peek_token_is(0, TokenType::Elif))
				return false;

			auto& elif_token = ref(0);
			auto offset = token_offset(1);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto& stmt = _tree.add(std::make_unique<ElifStatement>(std::move(elif_token), expr));
			cond.set_fallback(&stmt);
			auto ctx = context(stmt);
			parse_if_block(stmt);
			return true;
		}

		bool parse_else_statement(IfConditional& cond)
		{
			if (!peek_token_is(0, TokenType::Else))
				return false;

			auto& else_token = ref(0);
			token_offset(1).submit();
			auto& stmt = _tree.add(std::make_unique<ElseStatement>(std::move(else_token)));
			cond.set_fallback(&stmt);
			auto ctx = context(stmt);
			parse_simple_block(TokenType::If, errors::EXPECTED_IF_END, errors::EXPECTED_IF);
			return true;
		}

		bool parse_while_loop()
		{
			if (!peek_token_is(0, TokenType::While))
				return false;

			auto& loop_token = ref(0);
			auto offset = token_offset(1);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<WhileLoop>(std::move(loop_token), expr)));
			parse_simple_block(TokenType::While, errors::EXPECTED_WHILE_END, errors::EXPECTED_WHILE);
			return true;
		}

		bool parse_for_loop()
		{
			if (!peek_token_is(0, TokenType::For))
				return false;

			auto& loop_token = ref(0);
			auto& identifier = parse_token(1, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
			parse_token(2, TokenType::In, errors::EXPECTED_IN_CLAUSE);

			auto offset = token_offset(3);
			Expression& expr = parse_expression(offset);

			offset.submit();
			auto ctx = context(append_to_context(std::make_unique<ForLoop>(std::move(loop_token), std::move(identifier), expr)));
			parse_simple_block(TokenType::For, errors::EXPECTED_FOR_END, errors::EXPECTED_FOR);
			return true;
		}

		bool parse_log_statement()
		{
			if (!peek_token_is(0, TokenType::Log))
				return false;

			auto& log_token = ref(0);
			auto offset = token_offset(1);

			std::vector<Expression*> args;
			bool comma_ended = false;

			while (continue_statement())
			{
				args.push_back(&parse_expression(offset));
				comma_ended = false;
				if (peek_token_is_not(0, TokenType::Comma))
					break;
				else
				{
					offset.add(1);
					comma_ended = true;
				}
			}

			if (comma_ended || args.empty())
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			offset.submit();
			append_to_context(std::make_unique<LogStatement>(std::move(log_token), std::move(args)));
			return true;
		}

		bool parse_highlight_statement()
		{
			if (!peek_token_is(0, TokenType::Highlight))
				return false;

			auto& highlight_token = ref(0);
			auto offset = token_offset();
			offset.add(1);

			bool clear = false;
			if (peek_token_is(0, TokenType::Delete))
			{
				offset.add(1);
				clear = true;
			}

			Token* color_token = nullptr;
			BuiltinSymbol color = BuiltinSymbol::Yellow;
			Expression* expr = nullptr;

			if (peek_token_is_not(0, TokenType::Color))
				expr = &parse_expression(offset);

			if (peek_token_is(0, TokenType::Color))
			{
				color_token = &ref(0);
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
			append_to_context(std::make_unique<HighlightStatement>(std::move(highlight_token), clear, expr,
				color_token ? std::make_optional<Token>(*color_token) : std::nullopt, color));
			return true;
		}

		bool parse_end_statement(TokenType end, const char* missing_end_kw_err)
		{
			if (!peek_token_is(0, TokenType::End))
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

		void parse_if_block(IfConditional& cond)
		{
			while (!eof())
			{
				if (parse_end_statement(TokenType::If, errors::EXPECTED_IF))
					return;

				if (peek_token_is(0, TokenType::Elif) && parse_elif_statement(cond))
					return;

				if (peek_token_is(0, TokenType::Else) && parse_else_statement(cond))
					return;

				parse_statement();
			}

			throw_error(errors::EXPECTED_IF_END, 0);
		}

		Expression& parse_expression(TokenOffset& offset, Precedence min_precedence = Precedence::Lowest)
		{
			if (!continue_statement())
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			Expression* lhs = parse_wrapper_pattern_expression(offset);
			if (lhs)
				return *lhs;

			lhs = &parse_primary_expression(offset);

			while (continue_statement())
			{
				if (peek_token_is(0, TokenType::LBracket))
				{
					lhs = &parse_subscript_expression(*lhs, offset);
					continue;
				}
				
				if (peek_token_is(0, TokenType::Dot))
				{
					offset.add(1);
					auto& member = parse_token(0, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
					offset.add(1);
					MemberAccessExpression& expr = _tree.add(std::make_unique<MemberAccessExpression>(*lhs, std::move(member)));

					if (peek_token_is(0, TokenType::LParen))
						lhs = &parse_method_call_expression(expr, offset);
					else
						lhs = &expr;

					continue;
				}
				
				if (peek(0).is_postfix_operator())
				{
					if (!peek(0).is_binary_operator() || !(
						peek_token_is(1, TokenType::Identifier) ||
						peek_token_is(1, TokenType::BuiltinSymbol) ||
						peek_token_is(1, TokenType::Percent) ||
						peek_token_is(1, TokenType::LParen) ||
						peek_token_is(1, TokenType::LBracket) ||
						peek(1).is_literal() ||
						peek(1).is_prefix_operator()))
					{
						if (peek_token_is(0, TokenType::As))
							lhs = &parse_as_expression(*lhs, offset);
						else
							lhs = &parse_simple_repeat_expression(*lhs, offset);

						continue;
					}
				}
				
				if (peek(0).is_binary_operator())
				{
					Precedence precedence = peek(0).precedence();
					if (precedence < min_precedence)
						break;

					auto& op = ref(0);
					offset.add(1);
					Precedence next_min_precedence = precedence + (op.is_right_associative() ? 0 : 1);
					Expression& rhs = parse_expression(offset, next_min_precedence);
					lhs = &_tree.add(std::make_unique<BinaryExpression>(std::move(op), *lhs, rhs));

					continue;
				}

				break;
			}

			return *lhs;
		}

		Expression& parse_primary_expression(TokenOffset& offset)
		{
			if (peek_token_is(0, TokenType::Identifier))
				return parse_identifier_expression(offset);
			else if (peek_token_is(0, TokenType::BuiltinSymbol) || peek_token_is(0, TokenType::Percent))
				return parse_symbol_expression(offset);
			else if (peek(0).is_literal())
				return parse_literal_expression(offset);
			else if (peek_token_is(0, TokenType::LParen))
				return parse_group_expression(offset);
			else if (peek(0).is_prefix_operator())
				return parse_prefix_expression(offset);
			else if (peek_token_is(0, TokenType::LBracket))
				return parse_list_expression(offset);
			else
				throw_error(errors::UNRECOGNIZED_TOKEN, 0);
		}

		Expression& parse_identifier_expression(TokenOffset& offset)
		{
			auto& identifier = ref(0);
			offset.add(1);

			if (!peek_token_is(0, TokenType::LParen))
				return _tree.add(std::make_unique<VariableExpression>(std::move(identifier)));
			else
				return parse_function_call_expression(std::move(identifier), offset);
		}

		Expression& parse_function_call_expression(Token&& identifier, TokenOffset& offset)
		{
			auto arglist = parse_call_arglist(offset);
			return _tree.add(std::make_unique<FunctionCallExpression>(std::move(identifier), std::move(arglist), std::move(ref(-1))));
		}

		Expression& parse_method_call_expression(MemberAccessExpression& member, TokenOffset& offset)
		{
			auto arglist = parse_call_arglist(offset);
			return _tree.add(std::make_unique<MethodCallExpression>(member, std::move(arglist), std::move(ref(-1))));
		}

		std::vector<Expression*> parse_call_arglist(TokenOffset& offset)
		{
			offset.add(1);  // '('

			std::vector<Expression*> args;
			bool comma_ended = false;

			while (continue_statement() && peek_token_is_not(0, TokenType::RParen))
			{
				Expression& arg = parse_expression(offset, Precedence::Lowest);
				args.push_back(&arg);

				if (peek_token_is(0, TokenType::Comma))
				{
					comma_ended = true;
					offset.add(1);
				}
				else
				{
					comma_ended = false;
					break;
				}
			}

			if (comma_ended)
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			offset.add(1);  // ')'
			return args;
		}

		Expression& parse_symbol_expression(TokenOffset& offset)
		{
			auto symbol = parse_builtin_symbol(peek(0).lexeme);
			if (!symbol)
				throw_error(errors::UNRECOGNIZED_SYMBOL, 0);
			auto& symbol_token = ref(0);
			offset.add(1);
			return _tree.add(std::make_unique<BuiltinSymbolExpression>(std::move(symbol_token), *symbol));
		}

		Expression& parse_literal_expression(TokenOffset& offset)
		{
			auto& literal = ref(0);
			offset.add(1);
			return _tree.add(std::make_unique<LiteralExpression>(std::move(literal)));
		}

		Expression& parse_list_expression(TokenOffset& offset)
		{
			auto& lbracket_token = parse_token(0, TokenType::LBracket, errors::EXPECTED_LBRACKET);
			offset.add(1);  // '['

			std::vector<Expression*> elements;
			bool comma_ended = false;
			while (continue_statement() && !peek_token_is(0, TokenType::RBracket))
			{
				elements.push_back(&parse_expression(offset));

				if (peek_token_is(0, TokenType::Comma))
				{
					comma_ended = true;
					offset.add(1);
				}
				else
				{
					comma_ended = false;
					break;
				}
			}

			if (comma_ended)
				throw_error(errors::EXPECTED_EXPRESSION, 0);

			auto& rbracket_token = parse_token(0, TokenType::RBracket, errors::EXPECTED_RBRACKET);
			offset.add(1);  // ']'
			return _tree.add(std::make_unique<ListExpression>(std::move(lbracket_token), std::move(rbracket_token), std::move(elements)));
		}

		Expression& parse_group_expression(TokenOffset& offset)
		{
			offset.add(1);  // '('
			Expression& expr = parse_expression(offset, Precedence::Lowest);
			if (!peek_token_is(0, TokenType::RParen))
				throw_error(errors::EXPECTED_RPAREN, 0);
			offset.add(1);  // ')'
			return expr;
		}

		Expression& parse_subscript_expression(Expression& lhs, TokenOffset& offset)
		{
			offset.add(1);  // '['
			Expression& expr = parse_expression(offset, Precedence::Lowest);
			if (!peek_token_is(0, TokenType::RBracket))
				throw_error(errors::EXPECTED_RBRACKET, 0);
			offset.add(1);  // ']'
			return _tree.add(std::make_unique<SubscriptExpression>(lhs, expr));
		}

		Expression& parse_prefix_expression(TokenOffset& offset)
		{
			auto& op = ref(0);
			offset.add(1);
			Expression& rhs = parse_expression(offset, Precedence::Prefix);
			return _tree.add(std::make_unique<PrefixExpression>(std::move(op), rhs));
		}

		Expression& parse_as_expression(Expression& lhs, TokenOffset& offset)
		{
			offset.add(1);
			auto& datatype = parse_datatype(0);
			offset.add(1);
			return _tree.add(std::make_unique<AsExpression>(lhs, std::move(datatype)));
		}

		Expression& parse_simple_repeat_expression(Expression& lhs, TokenOffset& offset)
		{
			auto& op = ref(0);
			offset.add(1);
			return _tree.add(std::make_unique<SimpleRepeatOperation>(lhs, std::move(op)));
		}

		Expression* parse_wrapper_pattern_expression(TokenOffset& offset)
		{
			if (peek_token_is(0, TokenType::Capture))
			{
				auto& capture_token = ref(0);
				offset.add(1);
				auto& identifier = parse_token(0, TokenType::Identifier, errors::EXPECTED_IDENTIFIER);
				offset.add(1);
				return &_tree.add(std::make_unique<PatternCapture>(std::move(capture_token), std::move(identifier), parse_expression(offset, Precedence::Lowest)));
			}
			else if (peek_token_is(0, TokenType::Lazy))
			{
				auto& lazy_token = ref(0);
				offset.add(1);
				return &_tree.add(std::make_unique<PatternLazy>(std::move(lazy_token), parse_expression(offset, Precedence::Lowest)));
			}
			else
				return nullptr;
		}
	};

	Parser::Parser(Token&& start_token)
		: _tree(std::move(start_token))
	{
	}

	void Parser::parse(Lexer& lexer)
	{
		ASTBuilder builder(lexer.stream(), _errors, _tree);
	}

	const AbstractSyntaxTree& Parser::tree() const
	{
		return _tree;
	}

	AbstractSyntaxTree& Parser::tree()
	{
		return _tree;
	}

	const std::vector<LxError>& Parser::errors() const
	{
		return _errors;
	}
}
