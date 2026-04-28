#pragma once

#include <memory>

#include "token.h"
#include "symbols.h"
#include "errors.h"
#include "operations.h"
#include "runtime.h"

namespace lx
{
	class ASTNode;
	class Block;
	class ReturnStatement;

	struct ASTVisitor
	{
		virtual ~ASTVisitor() = default;
		virtual void pre_visit(const ASTNode& node) = 0;
		virtual void post_visit(const ASTNode& node) = 0;
	};

	struct UpflowInfo
	{
		bool all_return = false;
		std::vector<const ReturnStatement*> returns;
	};

	class ASTNode
	{
	protected:
		mutable bool _validated = false;
		mutable std::optional<UpflowInfo> _upflow;

	public:
		ASTNode() = default;
		ASTNode(const ASTNode&) = delete;
		virtual ~ASTNode() = default;
		
		bool validated() const;

		virtual void pre_analyse(RuntimeEnvironment& env) const = 0;
		virtual void post_analyse(RuntimeEnvironment& env) const = 0;
		void accept(ASTVisitor& visitor) const;
		virtual void traverse(ASTVisitor& visitor) const {}
		UpflowInfo upflow() const;

	protected:
		virtual UpflowInfo impl_upflow() const;
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		virtual void traverse(ASTVisitor& visitor) const override;

	protected:
		virtual UpflowInfo impl_upflow() const override;

	protected:
		virtual bool isolated() const = 0;

	public:
		void append(ASTNode& child);
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

		ASTNode& impl_add(std::unique_ptr<ASTNode>&& node);

	public:
		const ASTRoot& root() const;
		ASTRoot& root();

		template<typename T>
		T& add(std::unique_ptr<T>&& node) requires (std::is_base_of_v<ASTNode, T>)
		{
			T* n = node.get();
			impl_add(std::move(node));
			return *n;
		}
	};

	class Expression : public ASTNode
	{
		mutable std::optional<DataType> _evaltype;

	public:
		DataType evaltype(const RuntimeEnvironment& env) const;

	protected:
		virtual DataType impl_evaltype(const RuntimeEnvironment& env) const = 0;
	};

	class VariableDeclaration : public ASTNode
	{
		bool _global;
		Token _identifier;
		const Expression& _expression;

	public:
		VariableDeclaration(bool global, Token&& identifier, const Expression& expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
		
	public:
		bool is_global() const;
	};

	class VariableAssignment : public ASTNode
	{
		Token _identifier;
		const Expression& _expression;

	public:
		VariableAssignment(Token&& identifier, const Expression& expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	public:
		const std::string& variable_name() const;
	};

	class LiteralExpression : public Expression
	{
		Token _literal;

	public:
		LiteralExpression(Token&& literal);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class BinaryExpression : public Expression
	{
		Token _op;
		const Expression& _left;
		const Expression& _right;

	public:
		BinaryExpression(Token&& op, const Expression& left, const Expression& right);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;

	public:
		StandardBinaryOperator op() const;
	};

	class MemberAccessExpression : public Expression
	{
		const Expression& _object;
		Token _member;

	public:
		MemberAccessExpression(const Expression& object, Token&& member);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class PrefixExpression : public Expression
	{
		Token _op;
		const Expression& _expr;

	public:
		PrefixExpression(Token&& op, const Expression& expr);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;

	public:
		StandardPrefixOperator op() const;
	};

	class AsExpression : public Expression
	{
		const Expression& _expr;
		Token _type;

	public:
		AsExpression(const Expression& expr, Token&& type);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class SubscriptExpression : public Expression
	{
		const Expression& _container;
		const Expression& _subscript;

	public:
		SubscriptExpression(const Expression& container, const Expression& subscript);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class VariableExpression : public Expression
	{
		Token _identifier;

	public:
		VariableExpression(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class BuiltinSymbolExpression : public Expression
	{
		BuiltinSymbol _builtin_symbol;

	public:
		BuiltinSymbolExpression(BuiltinSymbol builtin_symbol);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class FunctionCallExpression : public Expression
	{
		Token _identifier;
		std::vector<const Expression*> _args;

	public:
		FunctionCallExpression(Token&& identifier, std::vector<const Expression*>&& args);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;

	public:
		std::vector<DataType> arg_types(const RuntimeEnvironment& env) const;
	};

	class MethodCallExpression : public Expression
	{
		const Expression& _object;
		std::vector<const Expression*> _args;

	public:
		MethodCallExpression(const Expression& object, std::vector<const Expression*>&& args);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		DataType impl_evaltype(const RuntimeEnvironment& env) const override;
	};

	class FunctionDefinition : public Block
	{
		Token _identifier;
		std::vector<std::pair<Token, Token>> _arglist;
		std::optional<Token> _return_type;

	public:
		FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, std::optional<Token>&& return_type);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;

	protected:
		UpflowInfo impl_upflow() const override;
		bool isolated() const override;

	public:
		std::vector<DataType> arg_types() const;
		std::vector<std::string_view> arg_identifiers() const;
		DataType return_type() const;
	};

	class ReturnStatement : public ASTNode
	{
		Token _return_token;
		const Expression* _expression;

	public:
		ReturnStatement(Token&& return_token, const Expression* expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		UpflowInfo impl_upflow() const override;
		
	public:
		DataType evaltype(const RuntimeEnvironment& env) const;
		const Token& return_token() const;
	};

	class IfFallbackBlock : public virtual Block
	{
	};
	
	class IfConditional : public virtual Block
	{
	protected:
		const IfFallbackBlock* _fallback = nullptr;

	public:
		void set_fallback(const IfFallbackBlock* fallback);

	protected:
		UpflowInfo impl_upflow() const override;
	};

	class IfStatement : public IfConditional
	{
		const Expression& _condition;

	public:
		IfStatement(const Expression& condition);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		UpflowInfo impl_upflow() const override;
		bool isolated() const override;
	};

	class ElifStatement : public IfConditional, public IfFallbackBlock
	{
		const Expression& _condition;

	public:
		ElifStatement(const Expression& condition);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		UpflowInfo impl_upflow() const override;
		bool isolated() const override;
	};

	class ElseStatement : public IfFallbackBlock
	{
	protected:
		bool isolated() const override;
	};

	class WhileLoop : public Block
	{
		const Expression& _condition;

	public:
		WhileLoop(const Expression& condition);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		bool isolated() const override;
	};

	class ForLoop : public Block
	{
		Token _iterator;
		const Expression& _iterable;

	public:
		ForLoop(Token&& iterator, const Expression& iterable);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

	protected:
		bool isolated() const override;
	};

	class BreakStatement : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env) const override {}
		void post_analyse(RuntimeEnvironment& env) const override {}
	};

	class ContinueStatement : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env) const override {}
		void post_analyse(RuntimeEnvironment& env) const override {}
	};

	class LogStatement : public ASTNode
	{
		std::vector<const Expression*> _args;

	public:
		LogStatement(std::vector<const Expression*>&& args);
		void pre_analyse(RuntimeEnvironment& env) const override {}
		void post_analyse(RuntimeEnvironment& env) const override {}
	};

	class HighlightStatement : public ASTNode
	{
		bool _clear;
		const Expression* _highlightable;
		BuiltinSymbol _color;

	public:
		HighlightStatement(bool clear, const Expression* highlightable, BuiltinSymbol color);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class DeletePattern : public ASTNode
	{
		Token _identifier;

	public:
		DeletePattern(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PatternDeclaration : public ASTNode
	{
		Token _identifier;

	public:
		PatternDeclaration(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PatternExpression : public ASTNode
	{
	};

	class PatternSubexpression : public PatternExpression
	{
		const Expression& _expr;

	public:
		PatternSubexpression(const Expression& expr);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternLiteral : public PatternExpression
	{
		Token _literal;

	public:
		PatternLiteral(Token&& literal);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PatternIdentifier : public PatternExpression
	{
		Token _identifier;

	public:
		PatternIdentifier(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PatternBuiltin : public PatternExpression
	{
		BuiltinSymbol _builtin_symbol;

	public:
		PatternBuiltin(BuiltinSymbol builtin_symbol);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PatternAs : public PatternExpression
	{
		const PatternExpression& _expression;
		Token _type;

	public:
		PatternAs(const PatternExpression& expression, Token&& type);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternRepeat : public PatternExpression
	{
		const PatternExpression& _expression;
		const Expression& _range;

	public:
		PatternRepeat(const PatternExpression& expression, const Expression& range);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternSimpleRepeat : public PatternExpression
	{
		const PatternExpression& _expression;
		Token _op;

	public:
		PatternSimpleRepeat(const PatternExpression& expression, Token&& op);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

		PatternSimpleRepeatOperator op() const;
	};

	class PatternPrefixOperation : public PatternExpression
	{
		Token _op;
		const PatternExpression& _expression;

	public:
		PatternPrefixOperation(Token&& op, const PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

		PatternPrefixOperator op() const;
	};

	class PatternBackRef : public PatternExpression
	{
		Token _identifier;

	public:
		PatternBackRef(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PatternBinaryOperation : public PatternExpression
	{
		Token _op;
		const PatternExpression& _left;
		const PatternExpression& _right;

	public:
		PatternBinaryOperation(Token&& op, const PatternExpression& left, const PatternExpression& right);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;

		PatternBinaryOperator op() const;
	};

	class PatternLazy : public PatternExpression
	{
		const PatternExpression& _expression;

	public:
		PatternLazy(const PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PatternCapture : public PatternExpression
	{
		Token _identifier;
		const PatternExpression& _expression;

	public:
		PatternCapture(Token&& identifier, const PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class AppendStatement : public ASTNode
	{
		const PatternExpression& _expression;

	public:
		AppendStatement(const PatternExpression& expression);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class FindStatement : public ASTNode
	{
		Token _identifier;

	public:
		FindStatement(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class FilterStatement : public ASTNode
	{
		Token _identifier;

	public:
		FilterStatement(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class ReplaceStatement : public ASTNode
	{
		const Expression& _match;
		const Expression& _string;

	public:
		ReplaceStatement(const Expression& match, const Expression& string);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class ApplyStatement : public ASTNode
	{
		Token _identifier;

	public:
		ApplyStatement(Token&& identifier);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class ScopeStatement : public ASTNode
	{
		Token _specifier;
		const Expression& _range;

	public:
		ScopeStatement(Token&& specifier, const Expression& range);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PagePush : public ASTNode
	{ 
		const Expression& _page;

	public:
		PagePush(const Expression& page);
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
		void traverse(ASTVisitor& visitor) const override;
	};

	class PagePop : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};

	class PageClearStack : public ASTNode
	{
	public:
		void pre_analyse(RuntimeEnvironment& env) const override;
		void post_analyse(RuntimeEnvironment& env) const override;
	};
}
