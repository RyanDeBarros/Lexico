#pragma once

#include <memory>

#include "token.h"
#include "symbols.h"
#include "errors.h"
#include "operations.h"
#include "resolution.h"

namespace lx
{
	class ASTNode;
	class Block;
	class ReturnStatement;
	class BreakStatement;
	class ContinueStatement;

	struct ASTVisitor
	{
		virtual ~ASTVisitor() = default;
		virtual void pre_visit(ASTNode& node) = 0;
		virtual void post_visit(ASTNode& node) = 0;
	};

	struct UpflowInfo
	{
		bool always_returns = false;
		bool may_break = false;
		bool may_continue = false;

		std::vector<const ReturnStatement*> live_returns;
		std::vector<const ReturnStatement*> dead_returns;
		std::vector<BreakStatement*> breaks;
		std::vector<ContinueStatement*> continues;

		void merge_loop_control(const UpflowInfo& other);
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

		virtual void pre_analyse(ResolutionContext& ctx) = 0;
		virtual void post_analyse(ResolutionContext& ctx) = 0;
		void accept(ASTVisitor& visitor);
		virtual void traverse(ASTVisitor& visitor) {}
		UpflowInfo upflow();

	protected:
		virtual UpflowInfo impl_upflow();
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		virtual void pre_analyse(ResolutionContext& ctx) override;
		virtual void post_analyse(ResolutionContext& ctx) override;
		virtual void traverse(ASTVisitor& visitor) override;

	protected:
		virtual UpflowInfo impl_upflow() override;

	protected:
		virtual bool isolated() const = 0;

	public:
		void append(ASTNode& child);
	};

	class ASTRoot : public Block
	{
	protected:
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
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
		mutable std::optional<ScriptSegment> _segment;

	public:
		DataType evaltype(const ResolutionContext& ctx) const;
		ScriptSegment segment() const;
		virtual bool imperative() const;

	protected:
		virtual DataType impl_evaltype(const ResolutionContext& ctx) const = 0;
		virtual ScriptSegment impl_segment() const = 0;
	};

	class VariableDeclaration : public ASTNode
	{
		bool _global;
		Token _identifier;
		Expression& _expression;

	public:
		VariableDeclaration(bool global, Token&& identifier, Expression& expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
		
	public:
		bool is_global() const;
	};

	class VariableAssignment : public ASTNode
	{
		Token _identifier;
		Expression& _expression;

	public:
		VariableAssignment(Token&& identifier, Expression& expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class LiteralExpression : public Expression
	{
		Token _literal;

	public:
		LiteralExpression(Token&& literal);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class ListExpression : public Expression
	{
		std::vector<Expression*> _elements;
		Token _lbracket_token;
		Token _rbracket_token;

	public:
		ListExpression(Token&& lbracket_token, Token&& rbracket_token, std::vector<Expression*>&& elements);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class BinaryExpression : public Expression
	{
		Token _op;
		Expression& _left;
		Expression& _right;

	public:
		BinaryExpression(Token&& op, Expression& left, Expression& right);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		StandardBinaryOperator op() const;
	};

	class MemberAccessExpression : public Expression
	{
		Expression& _object;
		Token _member;

	public:
		MemberAccessExpression(Expression& object, Token&& member);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		const MemberSignature& member(const ResolutionContext& ctx) const;
	};

	class PrefixExpression : public Expression
	{
		Token _op;
		Expression& _expr;

	public:
		PrefixExpression(Token&& op, Expression& expr);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		StandardPrefixOperator op() const;
	};

	class AsExpression : public Expression
	{
		Expression& _expr;
		Token _type;

	public:
		AsExpression(Expression& expr, Token&& type);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class SubscriptExpression : public Expression
	{
		Expression& _container;
		Expression& _subscript;

	public:
		SubscriptExpression(Expression& container, Expression& subscript);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		const MemberSignature& member(const ResolutionContext& ctx) const;
	};

	class VariableExpression : public Expression
	{
		Token _identifier;

	public:
		VariableExpression(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class BuiltinSymbolExpression : public Expression
	{
		Token _symbol_token;
		BuiltinSymbol _builtin_symbol;

	public:
		BuiltinSymbolExpression(Token&& symbol_token, BuiltinSymbol builtin_symbol);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class FunctionCallExpression : public Expression
	{
		Token _identifier;
		std::vector<Expression*> _args;
		Token _closing_paren;

	public:
		FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args, Token&& closing_paren);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		std::vector<DataType> arg_types(const ResolutionContext& ctx) const;
	};

	class MethodCallExpression : public Expression
	{
		MemberAccessExpression& _member;
		std::vector<Expression*> _args;
		Token _closing_paren;

	public:
		MethodCallExpression(MemberAccessExpression& member, std::vector<Expression*>&& args, Token&& closing_paren);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
		bool imperative() const override;

	protected:
		DataType impl_evaltype(const ResolutionContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class FunctionDefinition : public Block
	{
		Token _identifier;
		std::vector<std::pair<Token, Token>> _arglist;
		std::optional<Token> _return_type;

	public:
		FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, std::optional<Token>&& return_type);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		UpflowInfo impl_upflow() override;
		bool isolated() const override;

	public:
		std::vector<DataType> arg_types() const;
		std::vector<std::string_view> arg_identifiers() const;
		DataType return_type() const;
	};

	class ReturnStatement : public ASTNode
	{
		Token _return_token;
		Expression* _expression;

	public:
		ReturnStatement(Token&& return_token, Expression* expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		UpflowInfo impl_upflow() override;
		
	public:
		DataType evaltype(const ResolutionContext& ctx) const;
		ScriptSegment segment() const;
	};

	class IfFallbackBlock : public virtual Block
	{
	};
	
	class IfConditional : public virtual Block
	{
	protected:
		IfFallbackBlock* _fallback = nullptr;

	public:
		void set_fallback(IfFallbackBlock* fallback);

	protected:
		UpflowInfo impl_upflow() override;
	};

	class IfStatement : public IfConditional
	{
		Expression& _condition;

	public:
		IfStatement(Expression& condition);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		UpflowInfo impl_upflow() override;
		bool isolated() const override;
	};

	class ElifStatement : public IfConditional, public IfFallbackBlock
	{
		Expression& _condition;

	public:
		ElifStatement(Expression& condition);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		UpflowInfo impl_upflow() override;
		bool isolated() const override;
	};

	class ElseStatement : public IfFallbackBlock
	{
	protected:
		bool isolated() const override;
	};

	class Loop : public Block
	{

	protected:
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		UpflowInfo impl_upflow() override;
	};

	class WhileLoop : public Loop
	{
		Expression& _condition;

	public:
		WhileLoop(Expression& condition);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		bool isolated() const override;
	};

	class ForLoop : public Loop
	{
		Token _iterator;
		Expression& _iterable;

	public:
		ForLoop(Token&& iterator, Expression& iterable);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		bool isolated() const override;
	};

	class BreakStatement : public ASTNode
	{
		Token _break_token;
		const Loop* _backloop = nullptr;

	public:
		BreakStatement(Token&& break_token);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		UpflowInfo impl_upflow() override;

	public:
		void attach_loop(const Loop* loop);
		ScriptSegment segment() const;
	};

	class ContinueStatement : public ASTNode
	{
		Token _continue_token;
		const Loop* _backloop = nullptr;

	public:
		ContinueStatement(Token&& continue_token);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		UpflowInfo impl_upflow() override;

	public:
		void attach_loop(const Loop* loop);
		ScriptSegment segment() const;
	};

	class LogStatement : public ASTNode
	{
		std::vector<Expression*> _args;

	public:
		LogStatement(std::vector<Expression*>&& args);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class HighlightStatement : public ASTNode
	{
		bool _clear;
		Expression* _highlightable;
		std::optional<Token> _color_token;
		BuiltinSymbol _color;

	public:
		HighlightStatement(bool clear, Expression* highlightable, std::optional<Token>&& color_token, BuiltinSymbol color);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class DeletePattern : public ASTNode
	{
		Token _identifier;

	public:
		DeletePattern(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PatternDeclaration : public ASTNode
	{
		Token _identifier;

	public:
		PatternDeclaration(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PatternExpression : public ASTNode
	{
	};

	class PatternSubexpression : public PatternExpression
	{
		Expression& _expr;

	public:
		PatternSubexpression(Expression& expr);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class PatternLiteral : public PatternExpression
	{
		Token _literal;

	public:
		PatternLiteral(Token&& literal);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PatternIdentifier : public PatternExpression
	{
		Token _identifier;

	public:
		PatternIdentifier(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PatternBuiltin : public PatternExpression
	{
		Token _symbol_token;
		BuiltinSymbol _builtin_symbol;

	public:
		PatternBuiltin(Token&& symbol_token, BuiltinSymbol builtin_symbol);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PatternAs : public PatternExpression
	{
		PatternExpression& _expression;
		Token _type;

	public:
		PatternAs(PatternExpression& expression, Token&& type);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class PatternRepeat : public PatternExpression
	{
		PatternExpression& _expression;
		Expression& _range;

	public:
		PatternRepeat(PatternExpression& expression, Expression& range);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class PatternSimpleRepeat : public PatternExpression
	{
		PatternExpression& _expression;
		Token _op;

	public:
		PatternSimpleRepeat(PatternExpression& expression, Token&& op);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

		PatternSimpleRepeatOperator op() const;
	};

	class PatternPrefixOperation : public PatternExpression
	{
		Token _op;
		PatternExpression& _expression;

	public:
		PatternPrefixOperation(Token&& op, PatternExpression& expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

		PatternPrefixOperator op() const;
	};

	class PatternBackRef : public PatternExpression
	{
		Token _identifier;

	public:
		PatternBackRef(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PatternBinaryOperation : public PatternExpression
	{
		Token _op;
		PatternExpression& _left;
		PatternExpression& _right;

	public:
		PatternBinaryOperation(Token&& op, PatternExpression& left, PatternExpression& right);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

		PatternBinaryOperator op() const;
	};

	class PatternLazy : public PatternExpression
	{
		PatternExpression& _expression;

	public:
		PatternLazy(PatternExpression& expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class PatternCapture : public PatternExpression
	{
		Token _identifier;
		PatternExpression& _expression;

	public:
		PatternCapture(Token&& identifier, PatternExpression& expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class AppendStatement : public ASTNode
	{
		PatternExpression& _expression;

	public:
		AppendStatement(PatternExpression& expression);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class FindStatement : public ASTNode
	{
		Token _identifier;

	public:
		FindStatement(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class FilterStatement : public ASTNode
	{
		Token _identifier;

	public:
		FilterStatement(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class ReplaceStatement : public ASTNode
	{
		Expression& _match;
		Expression& _string;

	public:
		ReplaceStatement(Expression& match, Expression& string);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class ApplyStatement : public ASTNode
	{
		Token _identifier;

	public:
		ApplyStatement(Token&& identifier);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class ScopeStatement : public ASTNode
	{
		Token _symbol_token;
		BuiltinSymbol _specifier;
		Expression& _range;

	public:
		ScopeStatement(Token&& symbol_token, BuiltinSymbol specifier, Expression& range);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class PagePush : public ASTNode
	{ 
		Expression& _page;

	public:
		PagePush(Expression& page);
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;
	};

	class PagePop : public ASTNode
	{
	public:
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};

	class PageClearStack : public ASTNode
	{
	public:
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
	};
}
