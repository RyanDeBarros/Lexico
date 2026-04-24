#include "parser.h"

#include <stack>
#include <stdexcept>
#include <sstream>

namespace lx
{
	class ASTBuilder
	{
		TokenStream& _stream;
		size_t _token_offset = 0;
		AbstractSyntaxTree& _tree;
		std::stack<Block*> _context_stack;

	public:
		ASTBuilder(TokenStream& stream, AbstractSyntaxTree& tree)
			: _stream(stream), _tree(tree)
		{
			stream.seek();
			size_t tokens_left = SIZE_MAX;
			while (!_stream.eof() && _stream.tokens_left() < tokens_left)
			{
				parse_statement();
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

		bool statement_ends(size_t n)
		{
			return tokens_left() <= n || peek(n).type == TokenType::Newline || peek(n).type == TokenType::EndOfFile;
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

			// TODO parser error: unrecognized token
		}

		bool parse_pattern_declaration()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::PatternType)
				return false;

			if (tokens_left() < 2 || peek(1).type != TokenType::Identifier)
			{
				// TODO parser error: expected identifier
				return false;
			}

			if (!statement_ends(2))
			{
				// TODO parser error: unexpected token
				return false;
			}

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
			{
				// TODO parser error: expected identifier
				return false;
			}

			if (!statement_ends(2))
			{
				// TODO parser error: unexpected token
				return false;
			}

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
			{
				// TODO parser error: expected identifier
				return false;
			}

			if (!statement_ends(2))
			{
				// TODO parser error: unexpected token
				return false;
			}

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
			{
				// TODO parser error: expected predicate
				return false;
			}

			if (!statement_ends(2))
			{
				// TODO parser error: unexpected token
				return false;
			}

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
			std::unique_ptr<Expression> match_expr;
			size_t match_length;

			if (!parse_expression(match_expr, match_length))
			{
				// TODO parser error
				return false;
			}

			auto with_offset = token_offset(match_length);

			if (eof() || peek(0).type != TokenType::With)
			{
				// TODO parser error: expected 'with' clause
				return false;
			}

			auto string_offset = token_offset(1);
			std::unique_ptr<Expression> string_expr;
			size_t string_length;
			
			if (!parse_expression(string_expr, string_length))
			{
				// TODO parser error
				return false;
			}

			if (!statement_ends(string_length))
			{
				// TODO parser error: unexpected token
				return false;
			}

			append_to_context(std::make_unique<ReplaceStatement>(*match_expr, *string_expr));
			_tree.add(std::move(match_expr));
			_tree.add(std::move(string_expr));
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
			{
				// TODO parser error: expected predicate
				return false;
			}

			if (!statement_ends(2))
			{
				// TODO parser error: unexpected token
				return false;
			}

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
			{
				// TODO parser error: expected operand
				return false;
			}

			if (peek(1).type == TokenType::Push)
			{
				if (tokens_left() < 3)
				{
					// TODO parser error: missing page to push
					return false;
				}

				auto offset = token_offset(2);
				std::unique_ptr<Expression> page;
				size_t length;
				if (!parse_expression(page, length))
				{
					// TODO parser error: expected expression
					return false;
				}

				if (!statement_ends(length))
				{
					// TODO parser error: unexpected token
					return false;
				}

				append_to_context(std::make_unique<PagePush>(*page));
				_tree.add(std::move(page));
				advance(offset.offset() + length + 1);
				return true;
			}
			else if (peek(1).type == TokenType::Pop)
			{
				if (!statement_ends(2))
				{
					// TODO parser error: unexpected token
					return false;
				}

				append_to_context(std::make_unique<PagePop>());
				advance(3);
				return true;
			}
			else if (peek(1).type == TokenType::Delete)
			{
				if (!statement_ends(2))
				{
					// TODO parser error: unexpected token
					return false;
				}

				append_to_context(std::make_unique<PageClearStack>());
				advance(3);
				return true;
			}
			else
			{
				// TODO parser error: unrecognized operand
				return false;
			}
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
			{
				// TODO parser error: expected identifier
				return false;
			}

			if (peek(2).type != TokenType::Assign)
			{
				// TODO parser error: expected '='
				return false;
			}

			auto offset = token_offset(3);
			std::unique_ptr<Expression> expr;
			size_t length;

			if (!parse_expression(expr, length))
			{
				// TODO parser error
				return false;
			}

			bool global = peek(0).type == TokenType::Var;

			if (!statement_ends(length))
			{
				// TODO parser error: unexpected token
				return false;
			}

			append_to_context(std::make_unique<VariableDeclaration>(global, std::move(ref(1)), *expr));
			_tree.add(std::move(expr));
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
			{
				// TODO parser error: expected identifier
				return false;
			}

			auto offset = token_offset(2);
			std::unique_ptr<Expression> expr;
			size_t length;

			if (!parse_expression(expr, length))
			{
				// TODO parser error
				return false;
			}

			if (!statement_ends(length))
			{
				// TODO parser error: unexpected token
				return false;
			}

			append_to_context(std::make_unique<VariableAssignment>(std::move(ref(0)), *expr));
			_tree.add(std::move(expr));
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
			{
				// TODO parser error: expected operands
				return false;
			}

			auto offset = token_offset(1);

			std::vector<std::unique_ptr<Expression>> args_source;
			std::vector<Expression*> args;
			std::vector<TokenOffset> offsets;
			std::unique_ptr<Expression> expr;
			size_t length;
			bool comma_ended = false;
			while (parse_expression(expr, length))
			{
				args.push_back(expr.get());
				args_source.push_back(std::move(expr));
				expr = nullptr;
				offsets.push_back(token_offset(length));
				comma_ended = false;
				if (eof() || peek(0).type != TokenType::Comma)
					break;
				else
				{
					offsets.push_back(token_offset(1));
					comma_ended = true;
				}
			}

			if (comma_ended || args.empty())
			{
				// TODO parser error: expected expression
				return false;
			}

			if (!statement_ends(0))
			{
				// TODO parser error: unexpected token
				return false;
			}

			append_to_context(std::make_unique<LogStatement>(std::move(args)));
			for (auto& arg : args_source)
				_tree.add(std::move(arg));
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

			std::unique_ptr<Expression> expr = nullptr;
			size_t length = 0;
			if (!eof() && parse_expression(expr, length))
				offsets.push_back(token_offset(length));

			BuiltinSymbol color = BuiltinSymbol::Yellow;
			if (!eof() && peek(0).type == TokenType::Color)
			{
				offsets.push_back(token_offset(1));
				if (eof() || peek(0).type != TokenType::BuiltinSymbol)
				{
					// TODO parser error: expected color symbol
					return false;
				}

				if (auto c = parse_builtin_symbol(peek(0).lexeme))
				{
					color = *c;
					offsets.push_back(token_offset(1));
				}
				else
				{
					// TODO parser error: unrecognized color symbol
					return false;
				}
			}

			if (!statement_ends(0))
			{
				// TODO parser error: unexpected token
				return false;
			}

			append_to_context(std::make_unique<HighlightStatement>(clear, *expr, color));
			_tree.add(std::move(expr));
			advance(TokenOffset::offset(offsets) + 1);
			return true;
		}

		bool parse_expression(std::unique_ptr<Expression>& expr, size_t& length)
		{
			expr = nullptr;
			length = 0;

			// TODO some expressions move tokens from the stream - but if expression is ultimately not added, they need to be returned to the stream. Otherwise, make expression nodes use token copies

			// TODO
			return false;
		}
	};

	void Parser::parse(TokenStream& stream)
	{
		ASTBuilder builder(stream, _tree);
	}

	const AbstractSyntaxTree& Parser::tree() const
	{
		return _tree;
	}

	AbstractSyntaxTree& Parser::tree()
	{
		return _tree;
	}
}
