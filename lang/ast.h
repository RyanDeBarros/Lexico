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
		mutable std::optional<ScriptSegment> _segment;

	public:
		ASTNode() = default;
		ASTNode(const ASTNode&) = delete;
		virtual ~ASTNode() = default;
		
		bool validated() const;

		virtual void pre_analyse(ResolutionContext& ctx) = 0;
		virtual void post_analyse(ResolutionContext& ctx) = 0;
		void accept(ASTVisitor& visitor);
		virtual void traverse(ASTVisitor& visitor) {}
		UpflowInfo upflow(const ResolutionContext& ctx);
		ScriptSegment segment() const;

	protected:
		virtual UpflowInfo impl_upflow(const ResolutionContext& ctx);
		virtual ScriptSegment impl_segment() const = 0;
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		virtual void pre_analyse(ResolutionContext& ctx) override;
		virtual void post_analyse(ResolutionContext& ctx) override;
		virtual void traverse(ASTVisitor& visitor) override;

	protected:
		virtual UpflowInfo impl_upflow(const ResolutionContext& ctx) override;

	protected:
		virtual bool isolated() const = 0;

	public:
		void append(ASTNode& child);
	};

	class ASTRoot : public Block
	{
		Token _start_token;

	public:
		ASTRoot(Token&& start_token);

	protected:
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		bool isolated() const override;
		ScriptSegment impl_segment() const override;
	};

	class AbstractSyntaxTree
	{
		ASTRoot _root;
		std::vector<std::unique_ptr<ASTNode>> _nodes;

		ASTNode& impl_add(std::unique_ptr<ASTNode>&& node);

	public:
		AbstractSyntaxTree(Token&& start_token);

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
		DataType evaltype(const ResolutionContext& ctx) const;
		virtual bool imperative() const;

	protected:
		virtual DataType impl_evaltype(const ResolutionContext& ctx) const = 0;
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

	protected:
		ScriptSegment impl_segment() const override;
		
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

	protected:
		ScriptSegment impl_segment() const override;
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
		bool _callable = false;

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
		void set_callable(bool callable);
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
		Token _fn_token;
		Token _identifier;
		std::vector<std::pair<Token, Token>> _arglist;
		std::optional<Token> _return_type;

	public:
		FunctionDefinition(Token&& fn_token, Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, std::optional<Token>&& return_type);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;
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
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;
		
	public:
		DataType evaltype(const ResolutionContext& ctx) const;
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
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
	};

	class IfStatement : public IfConditional
	{
		Token _if_token;
		Expression& _condition;

	public:
		IfStatement(Token&& if_token, Expression& condition);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;
		bool isolated() const override;
	};

	class ElifStatement : public IfConditional, public IfFallbackBlock
	{
		Token _elif_token;
		Expression& _condition;

	public:
		ElifStatement(Token&& elif_token, Expression& condition);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;
		bool isolated() const override;
	};

	class ElseStatement : public IfFallbackBlock
	{
		Token _else_token;

	public:
		ElseStatement(Token&& else_token);

	protected:
		ScriptSegment impl_segment() const override;
		bool isolated() const override;
	};

	class Loop : public Block
	{
		Token _loop_token;

	public:
		Loop(Token&& loop_token);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;
	};

	class WhileLoop : public Loop
	{
		Expression& _condition;

	public:
		WhileLoop(Token&& loop_token, Expression& condition);

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
		ForLoop(Token&& loop_token, Token&& iterator, Expression& iterable);

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
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;

	public:
		void attach_loop(const Loop* loop);
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
		UpflowInfo impl_upflow(const ResolutionContext& ctx) override;
		ScriptSegment impl_segment() const override;

	public:
		void attach_loop(const Loop* loop);
	};

	class LogStatement : public ASTNode
	{
		Token _log_token;
		std::vector<Expression*> _args;

	public:
		LogStatement(Token&& log_token, std::vector<Expression*>&& args);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class HighlightStatement : public ASTNode
	{
		Token _highlight_token;
		bool _clear;
		Expression* _highlightable;
		std::optional<Token> _color_token;
		BuiltinSymbol _color;

	public:
		HighlightStatement(Token&& highlight_token, bool clear, Expression* highlightable, std::optional<Token>&& color_token, BuiltinSymbol color);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class DeletePattern : public ASTNode
	{
		Token _delete_token;
		Token _identifier;

	public:
		DeletePattern(Token&& delete_token, Token&& identifier);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PatternDeclaration : public ASTNode
	{
		Token _pattern_token;
		Token _identifier;

	public:
		PatternDeclaration(Token&& pattern_token, Token&& identifier);
		
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
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

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PatternLiteral : public PatternExpression
	{
		Token _literal;

	public:
		PatternLiteral(Token&& literal);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PatternIdentifier : public PatternExpression
	{
		Token _identifier;

	public:
		PatternIdentifier(Token&& identifier);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PatternBuiltin : public PatternExpression
	{
		Token _symbol_token;
		BuiltinSymbol _builtin_symbol;

	public:
		PatternBuiltin(Token&& symbol_token, BuiltinSymbol builtin_symbol);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
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

	protected:
		ScriptSegment impl_segment() const override;
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

	protected:
		ScriptSegment impl_segment() const override;
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

	protected:
		ScriptSegment impl_segment() const override;

	public:
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

	protected:
		ScriptSegment impl_segment() const override;

	public:
		PatternPrefixOperator op() const;
	};

	class PatternBackRef : public PatternExpression
	{
		Token _ref_token;
		Token _identifier;

	public:
		PatternBackRef(Token&& ref_token, Token&& identifier);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
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

	protected:
		ScriptSegment impl_segment() const override;

	public:
		PatternBinaryOperator op() const;
	};

	class PatternLazy : public PatternExpression
	{
		Token _lazy_token;
		PatternExpression& _expression;

	public:
		PatternLazy(Token&& lazy_token, PatternExpression& expression);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PatternCapture : public PatternExpression
	{
		Token _capture_token;
		Token _identifier;
		PatternExpression& _expression;

	public:
		PatternCapture(Token&& capture_token, Token&& identifier, PatternExpression& expression);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class AppendStatement : public ASTNode
	{
		Token _append_token;
		PatternExpression& _expression;

	public:
		AppendStatement(Token&& append_token, PatternExpression& expression);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class FindStatement : public ASTNode
	{
		Token _find_token;
		Token _identifier;

	public:
		FindStatement(Token&& find_token, Token&& identifier);
		
		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class FilterStatement : public ASTNode
	{
		Token _filter_token;
		Token _identifier;

	public:
		FilterStatement(Token&& filter_token, Token&& identifier);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class ReplaceStatement : public ASTNode
	{
		Token _replace_token;
		Expression& _match;
		Expression& _string;

	public:
		ReplaceStatement(Token&& replace_token, Expression& match, Expression& string);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class ApplyStatement : public ASTNode
	{
		Token _apply_token;
		Token _identifier;

	public:
		ApplyStatement(Token&& apply_token, Token&& identifier);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class ScopeStatement : public ASTNode
	{
		Token _scope_token;
		Token _symbol_token;
		BuiltinSymbol _specifier;
		Expression& _range;

	public:
		ScopeStatement(Token&& scope_token, Token&& symbol_token, BuiltinSymbol specifier, Expression& range);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PagePush : public ASTNode
	{
		Token _page_token;
		Expression& _page;

	public:
		PagePush(Token&& page_token, Expression& page);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;
		void traverse(ASTVisitor& visitor) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PagePop : public ASTNode
	{
		Token _page_token;

	public:
		PagePop(Token&& page_token);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};

	class PageClearStack : public ASTNode
	{
		Token _page_token;

	public:
		PageClearStack(Token&& page_token);

		void pre_analyse(ResolutionContext& ctx) override;
		void post_analyse(ResolutionContext& ctx) override;

	protected:
		ScriptSegment impl_segment() const override;
	};
}
