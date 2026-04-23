#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	ASTNode& AbstractSyntaxTree::add(std::unique_ptr<ASTNode>&& node)
	{
		ASTNode* ref = node.get();
		_nodes.push_back(std::move(node));
		return *ref;
	}

	const ASTNode& AbstractSyntaxTree::root() const
	{
		return *_nodes[0];
	}

	ASTNode& AbstractSyntaxTree::root()
	{
		return *_nodes[0];
	}

	void Block::append(ASTNode& child)
	{
		_children.push_back(&child);
	}

	VariableDeclaration::VariableDeclaration(Token&& declarer, Token&& identifier, Expression& expression)
		: _declarer(std::move(declarer)), _identifier(std::move(identifier)), _expression(&expression)
	{
	}

	bool VariableDeclaration::is_global() const
	{
		return _declarer.type == TokenType::Var;
	}

	const std::string& VariableDeclaration::variable_name() const
	{
		return _identifier.lexeme;
	}

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	BinaryExpression::BinaryExpression(Token&& op, Expression& left, Expression& right)
		: _op(std::move(op)), _left(&left), _right(&right)
	{
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	FunctionExpression::FunctionExpression(Token&& identifier, std::vector<Expression*>&& args)
		: _identifier(std::move(identifier)), _args(std::move(args))
	{
	}
}
