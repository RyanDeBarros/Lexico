#pragma once

#include <memory>

#include "token.h"
#include "types.h"

namespace lx
{
	struct ASTNode
	{
		virtual ~ASTNode() = default;
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		void append(ASTNode& child);
	};

	class ASTRoot : public Block
	{
	};

	class AbstractSyntaxTree
	{
		ASTRoot _root;
		std::vector<std::unique_ptr<ASTNode>> _nodes;

		ASTNode& impl_add(std::unique_ptr<ASTNode>&& node);

	public:
		const ASTRoot& root() const;
		ASTRoot& root();

		template<typename T>
		T& add(std::unique_ptr<T>&& node) requires (std::is_base_of_v<ASTNode, T>)
		{
			return static_cast<T&>(impl_add(std::move(node)));
		}
	};

	struct Expression : public ASTNode
	{
	};

	class VariableDeclaration : public ASTNode
	{
		bool _global;
		Token _identifier;
		Expression* _expression;

	public:
		VariableDeclaration(bool global, Token&& identifier, Expression& expression);
		
		bool is_global() const;
		const std::string& variable_name() const;
	};

	class VariableAssignment : public ASTNode
	{
		Token _identifier;
		Expression* _expression;

	public:
		VariableAssignment(Token&& identifier, Expression& expression);

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

	struct PercentExpression : public Expression
	{
	};

	class FunctionCallExpression : public Expression
	{
		Token _identifier;
		std::vector<Expression*> _args;

	public:
		FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args);
	};

	class FunctionDefinition : public Block
	{
		Token _identifier;
		std::vector<std::pair<Token, Token>> _arglist;
		Token _return_type;

	public:
		FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type);
	};

	class ReturnStatement : public ASTNode
	{
		Expression* _expression;

	public:
		ReturnStatement(Expression* expression);
	};

	class IfStatement : public Block
	{
		Expression* _condition;

	public:
		IfStatement(Expression* condition);
	};

	class ElifStatement : public Block
	{
		Expression* _condition;

	public:
		ElifStatement(Expression* condition);
	};

	class ElseStatement : public Block
	{
	};

	class WhileLoopStatement : public Block
	{
		Expression* _condition;

	public:
		WhileLoopStatement(Expression* condition);
	};

	class ForLoopStatement : public Block
	{
		Token _iterator;
		Expression* _iterable;

	public:
		ForLoopStatement(Token&& iterator, Expression* iterable);
	};

	class LogStatement : public ASTNode
	{
		std::vector<Expression*> _args;

	public:
		LogStatement(std::vector<Expression*>&& args);
	};

	class HighlightStatement : public ASTNode
	{
		bool _clear;
		Expression* _highlightable;
		std::optional<Token> _color;

	public:
		HighlightStatement(bool clear, Expression* highlightable, std::optional<Token>&& color);
	};

	class DeletePattern : public ASTNode
	{
		Token _identifier;

	public:
		DeletePattern(Token&& identifier);
	};

	class PatternDeclaration : public ASTNode
	{
		Token _identifier;

	public:
		PatternDeclaration(Token&& identifier);
	};

	struct PatternExpression : public Expression
	{
	};

	class PatternLiteral : public PatternExpression
	{
		Token _literal;

	public:
		PatternLiteral(Token&& literal);
	};

	class PatternIdentifier : public PatternExpression
	{
		Token _identifier;

	public:
		PatternIdentifier(Token&& identifier);
	};

	class PatternBuiltin : public PatternExpression
	{
		Token _builtin_symbol;

	public:
		PatternBuiltin(Token&& builtin_symbol);
	};

	class PatternAs : public PatternExpression
	{
		PatternExpression* _expression;
		Token _type;

	public:
		PatternAs(PatternExpression* expression, Token&& type);
	};

	class PatternRepeat : public PatternExpression
	{
		PatternExpression* _expression;
		Expression* _range;

	public:
		PatternRepeat(PatternExpression* expression, Expression* range);
	};

	class PatternSimpleRepeat : public PatternExpression
	{
		PatternExpression* _expression;
		Token _op;

	public:
		PatternSimpleRepeat(PatternExpression* expression, Token&& op);
	};

	class PatternPrefixOperation : public PatternExpression
	{
		Token _op;
		PatternExpression* _expression;

	public:
		PatternPrefixOperation(Token&& op, PatternExpression* expression);
	};

	class PatternBackRef : public PatternExpression
	{
		Token _identifier;

	public:
		PatternBackRef(Token&& identifier);
	};

	class PatternBinaryOperation : public PatternExpression
	{
		Token _op;
		PatternExpression* _left;
		PatternExpression* _right;

	public:
		PatternBinaryOperation(Token&& op, PatternExpression* left, PatternExpression* right);
	};

	class PatternLazy : public PatternExpression
	{
		PatternExpression* _expression;

	public:
		PatternLazy(PatternExpression* expression);
	};

	class PatternCapture : public PatternExpression
	{
		Token _identifier;
		PatternExpression* _expression;

	public:
		PatternCapture(Token&& identifier, PatternExpression* expression);
	};

	class AppendStatement : public ASTNode
	{
		PatternExpression* _expression;

	public:
		AppendStatement(PatternExpression* expression);
	};

	class FindStatement : public ASTNode
	{
		Token _identifier;

	public:
		FindStatement(Token&& identifier);
	};

	class FilterStatement : public ASTNode
	{
		Token _identifier;

	public:
		FilterStatement(Token&& identifier);
	};

	class ReplaceStatement : public ASTNode
	{
		Token _identifier;
		Expression* _string;

	public:
		ReplaceStatement(Token&& identifier, Expression* string);
	};

	class ApplyStatement : public ASTNode
	{
		Token _identifier;

	public:
		ApplyStatement(Token&& identifier);
	};

	class ScopeStatement : public ASTNode
	{
		Token _specifier;
		Expression* _range;

	public:
		ScopeStatement(Token&& specifier, Expression* range);
	};

	class PagePush : public ASTNode
	{ 
		Expression* _page;

	public:
		PagePush(Expression* page);
	};

	class PagePop : public ASTNode
	{
	};

	class PageClearStack : public ASTNode
	{
	};
}
