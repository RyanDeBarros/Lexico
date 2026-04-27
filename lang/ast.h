#pragma once

#include <memory>

#include "token.h"
#include "symbols.h"
#include "errors.h"
#include "runtime.h"

namespace lx
{
	class ASTNode;

	struct ASTVisitor
	{
		virtual ~ASTVisitor() = default;
		virtual void pre_visit(const ASTNode& node) = 0;
		virtual void post_visit(const ASTNode& node) = 0;
	};

	class ASTNode
	{
	public:
		ASTNode() = default;
		ASTNode(const ASTNode&) = delete;
		virtual ~ASTNode() = default;
		
		virtual void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const = 0;
		virtual void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const = 0;
		void accept(ASTVisitor& visitor) const;
		virtual void traverse(ASTVisitor& visitor) const {}
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		virtual void traverse(ASTVisitor& visitor) const override;

	protected:
		virtual bool isolated() const = 0;

	public:
		void append(ASTNode& child);

	protected:
	};

	class ASTRoot : public Block
	{
	protected:
		bool isolated() const override;
	};

	class AbstractSyntaxTree
	{
		ASTRoot _root;
		std::vector<std::unique_ptr<ASTNode>> _nodes;

		ASTNode& add_impl(std::unique_ptr<ASTNode>&& node);

	public:
		const ASTRoot& root() const;
		ASTRoot& root();

		template<typename T>
		T& add(std::unique_ptr<T>&& node) requires (std::is_base_of_v<ASTNode, T>)
		{
			T* n = node.get();
			add_impl(std::move(node));
			return *n;
		}
	};

	class Expression : public ASTNode
	{
	};

	class VariableDeclaration : public ASTNode
	{
		bool _global;
		Token _identifier;
		Expression& _expression;

	public:
		VariableDeclaration(bool global, Token&& identifier, Expression& expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
		
	public:
		bool is_global() const;
		std::string_view variable_name() const;
	};

	class VariableAssignment : public ASTNode
	{
		Token _identifier;
		Expression& _expression;

	public:
		VariableAssignment(Token&& identifier, Expression& expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;

	public:
		const std::string& variable_name() const;
	};

	class LiteralExpression : public Expression
	{
		Token _literal;

	public:
		LiteralExpression(Token&& literal);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class BinaryExpression : public Expression
	{
		Token _op;
		Expression& _left;
		Expression& _right;

	public:
		BinaryExpression(Token&& op, Expression& left, Expression& right);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PrefixExpression : public Expression
	{
		Token _op;
		Expression& _expr;

	public:
		PrefixExpression(Token&& op, Expression& expr);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class AsExpression : public Expression
	{
		Expression& _expr;
		Token _type;

	public:
		AsExpression(Expression& expr, Token&& type);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class SubscriptExpression : public Expression
	{
		Expression& _container;
		Expression& _subscript;

	public:
		SubscriptExpression(Expression& container, Expression& subscript);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class VariableExpression : public Expression
	{
		Token _identifier;

	public:
		VariableExpression(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class BuiltinSymbolExpression : public Expression
	{
		BuiltinSymbol _builtin_symbol;

	public:
		BuiltinSymbolExpression(BuiltinSymbol builtin_symbol);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class FunctionCallExpression : public Expression
	{
		Token _identifier;
		std::vector<Expression*> _args;

	public:
		FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class FunctionDefinition : public Block
	{
		Token _identifier;
		std::vector<std::pair<Token, Token>> _arglist;
		Token _return_type;

	public:
		FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;

	protected:
		bool isolated() const override;
	};

	class ReturnStatement : public ASTNode
	{
		Expression* _expression;

	public:
		ReturnStatement(Expression* expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class IfStatement : public Block
	{
		Expression& _condition;

	public:
		IfStatement(Expression& condition);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		bool isolated() const override;
	};

	class ElifStatement : public Block
	{
		Expression& _condition;

	public:
		ElifStatement(Expression& condition);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		bool isolated() const override;
	};

	class ElseStatement : public Block
	{
	protected:
		bool isolated() const override;
	};

	class WhileLoop : public Block
	{
		Expression& _condition;

	public:
		WhileLoop(Expression& condition);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		bool isolated() const override;
	};

	class ForLoop : public Block
	{
		Token _iterator;
		Expression& _iterable;

	public:
		ForLoop(Token&& iterator, Expression& iterable);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		bool isolated() const override;
	};

	class BreakStatement : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override {}
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override {}
	};

	class ContinueStatement : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override {}
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override {}
	};

	class LogStatement : public ASTNode
	{
		std::vector<Expression*> _args;

	public:
		LogStatement(std::vector<Expression*>&& args);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override {}
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override {}
	};

	class HighlightStatement : public ASTNode
	{
		bool _clear;
		Expression* _highlightable;
		BuiltinSymbol _color;

	public:
		HighlightStatement(bool clear, Expression* highlightable, BuiltinSymbol color);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class DeletePattern : public ASTNode
	{
		Token _identifier;

	public:
		DeletePattern(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PatternDeclaration : public ASTNode
	{
		Token _identifier;

	public:
		PatternDeclaration(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PatternExpression : public ASTNode
	{
	};

	class PatternSubexpression : public PatternExpression
	{
		Expression& _expr;

	public:
		PatternSubexpression(Expression& expr);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternLiteral : public PatternExpression
	{
		Token _literal;

	public:
		PatternLiteral(Token&& literal);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PatternIdentifier : public PatternExpression
	{
		Token _identifier;

	public:
		PatternIdentifier(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PatternBuiltin : public PatternExpression
	{
		BuiltinSymbol _builtin_symbol;

	public:
		PatternBuiltin(BuiltinSymbol builtin_symbol);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PatternAs : public PatternExpression
	{
		PatternExpression& _expression;
		Token _type;

	public:
		PatternAs(PatternExpression& expression, Token&& type);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternRepeat : public PatternExpression
	{
		PatternExpression& _expression;
		Expression& _range;

	public:
		PatternRepeat(PatternExpression& expression, Expression& range);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternSimpleRepeat : public PatternExpression
	{
		PatternExpression& _expression;
		Token _op;

	public:
		PatternSimpleRepeat(PatternExpression& expression, Token&& op);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternPrefixOperation : public PatternExpression
	{
		Token _op;
		PatternExpression& _expression;

	public:
		PatternPrefixOperation(Token&& op, PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternBackRef : public PatternExpression
	{
		Token _identifier;

	public:
		PatternBackRef(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PatternBinaryOperation : public PatternExpression
	{
		Token _op;
		PatternExpression& _left;
		PatternExpression& _right;

	public:
		PatternBinaryOperation(Token&& op, PatternExpression& left, PatternExpression& right);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternLazy : public PatternExpression
	{
		PatternExpression& _expression;

	public:
		PatternLazy(PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternCapture : public PatternExpression
	{
		Token _identifier;
		PatternExpression& _expression;

	public:
		PatternCapture(Token&& identifier, PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class AppendStatement : public ASTNode
	{
		PatternExpression& _expression;

	public:
		AppendStatement(PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class FindStatement : public ASTNode
	{
		Token _identifier;

	public:
		FindStatement(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class FilterStatement : public ASTNode
	{
		Token _identifier;

	public:
		FilterStatement(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class ReplaceStatement : public ASTNode
	{
		Expression& _match;
		Expression& _string;

	public:
		ReplaceStatement(Expression& match, Expression& string);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class ApplyStatement : public ASTNode
	{
		Token _identifier;

	public:
		ApplyStatement(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class ScopeStatement : public ASTNode
	{
		Token _specifier;
		Expression& _range;

	public:
		ScopeStatement(Token&& specifier, Expression& range);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PagePush : public ASTNode
	{ 
		Expression& _page;

	public:
		PagePush(Expression& page);
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PagePop : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};

	class PageClearStack : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
		void post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const override;
	};
}
