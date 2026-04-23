#pragma once

#include <memory>

#include "token.h"

namespace lx
{
	struct ASTNode
	{
		virtual ~ASTNode() = default;
	};

	class AbstractSyntaxTree
	{
		std::vector<std::unique_ptr<ASTNode>> _nodes;

	public:
		ASTNode& add(std::unique_ptr<ASTNode>&& node);
		const ASTNode& root() const;
		ASTNode& root();
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		void append(ASTNode& child);
	};

	struct Expression : public ASTNode
	{
		// TODO virtual value variant
	};

	class VariableDeclaration : public ASTNode
	{
		Token _declarer;
		Token _identifier;
		Expression* _expression;

	public:
		VariableDeclaration(Token&& declarer, Token&& identifier, Expression& expression);
		
		bool is_global() const;
		const std::string& variable_name() const;
	};

	class LiteralExpression : public Expression
	{
		Token _literal;

	public:
		LiteralExpression(Token&& literal);
	};

	class BinaryExpression : public Expression
	{
		Token _op;
		Expression* _left;
		Expression* _right;

	public:
		BinaryExpression(Token&& op, Expression& left, Expression& right);
	};

	class VariableExpression : public Expression
	{
		Token _identifier;

	public:
		VariableExpression(Token&& identifier);
	};

	class FunctionExpression : public Expression
	{
		Token _identifier;
		std::vector<Expression*> _args;

	public:
		FunctionExpression(Token&& identifier, std::vector<Expression*>&& args);
	};
}
