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
				if (!parse_statement())
					break; // TODO throw error?
				tokens_left = _stream.tokens_left();
			}
		}

	private:
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
			if (_token_offset > 0)
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": token offset is non-zero (" << _token_offset << ")";
				throw std::runtime_error(ss.str());
			}

			_stream.advance(n);
		}

		bool eof() const
		{
			return _stream.eof();
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

		bool parse_statement()
		{
			if (parse_pattern_declaration())
				return true;

			if (parse_delete_pattern())
				return true;

			if (parse_append_statement())
				return true;

			if (parse_scope_statement())
				return true;

			if (parse_find_statement())
				return true;

			if (parse_filter_statement())
				return true;

			if (parse_replace_statement())
				return true;

			if (parse_apply_statement())
				return true;

			if (parse_page_statement())
				return true;

			if (parse_function_definition())
				return true;

			if (parse_variable_declaration())
				return true;

			if (parse_assignment())
				return true;

			if (parse_control_statement())
				return true;

			if (parse_log_statement())
				return true;

			if (parse_highlight_statement())
				return true;

			return false;
		}

		bool parse_pattern_declaration()
		{
			// TODO
			return false;
		}

		bool parse_delete_pattern()
		{
			if (eof())
				return false;

			if (peek(0).type != TokenType::Delete)
				return false;

			if (tokens_left() < 2)
			{
				// TODO parser error: excepted identifier
				return false;
			}

			if (peek(1).type != TokenType::Identifier)
			{
				// TODO parser error: expected identifier
				return false;
			}

			append_to_context(std::make_unique<DeletePattern>(std::move(ref(1))));
			advance(2);
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
			// TODO
			return false;
		}

		bool parse_filter_statement()
		{
			// TODO
			return false;
		}

		bool parse_replace_statement()
		{
			// TODO
			return false;
		}

		bool parse_apply_statement()
		{
			// TODO
			return false;
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

				_token_offset += 2;
				Expression* page = nullptr;
				size_t length = 0;
				bool parsed = parse_expression(page, length);
				_token_offset -= 2;
				if (!parsed)
				{
					// TODO parser error: expected expression
					return false;
				}

				append_to_context(std::make_unique<PagePush>(page));
				advance(2 + length);
				return true;
			}
			else if (peek(1).type == TokenType::Pop)
			{
				append_to_context(std::make_unique<PagePop>());
				advance(2);
				return true;
			}
			else if (peek(1).type == TokenType::Delete)
			{
				append_to_context(std::make_unique<PageClearStack>());
				advance(2);
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

			_token_offset += 3;
			Expression* expr = nullptr;
			size_t length = 0;
			bool parsed = parse_expression(expr, length);
			_token_offset -= 3;

			if (!parsed)
			{
				// TODO parser error
				return false;
			}

			bool global = peek(0).type == TokenType::Var;

			append_to_context(std::make_unique<VariableDeclaration>(global, std::move(ref(1)), *expr));
			advance(3 + length);
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

			_token_offset += 2;
			Expression* expr = nullptr;
			size_t length = 0;
			bool parsed = parse_expression(expr, length);
			_token_offset -= 2;

			if (!parsed)
			{
				// TODO parser error
				return false;
			}

			append_to_context(std::make_unique<VariableAssignment>(std::move(ref(0)), *expr));
			advance(2 + length);
			return true;
		}

		bool parse_control_statement()
		{
			// TODO
			return false;
		}

		bool parse_log_statement()
		{
			// TODO
			return false;
		}

		bool parse_highlight_statement()
		{
			// TODO
			return false;
		}

		bool parse_expression(Expression*& expr, size_t& length)
		{
			expr = nullptr;
			length = 1;
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
