#pragma once

#include <memory>

#include "token.h"
#include "symbols.h"
#include "errors.h"
#include "operations.h"
#include "resolution.h"
#include "runtime.h"
#include "types/datapoint.h"
#include "types/member.h"

namespace lx
{
	class FullTypeKeyword
	{
		TokenType _simple;
		ScriptSegment _segment;
		std::vector<TokenType> _underlying;

	public:
		FullTypeKeyword(const Token& simple, const std::vector<Token>& underlying);

		DataType type() const;
		const ScriptSegment& segment() const;
	};

	class ReturnStatement;
	class BreakStatement;
	class ContinueStatement;

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

	enum class FlowType
	{
		Normal,
		Return,
		Break,
		Continue,
	};

	struct ExecutionFlow
	{
		FlowType type = FlowType::Normal;
		std::optional<Variable> data = std::nullopt;
	};

	struct InvokeResult
	{
		Variable data;
	};

	enum class AnalysisPass
	{
		Registration,
		Validation,
		VarConsistencySetup,
		VarConsistencyExec,
	};

	class ASTNode
	{
	protected:
		mutable std::optional<UpflowInfo> _upflow;
		mutable std::optional<ScriptSegment> _segment;

	public:
		ASTNode() = default;
		ASTNode(const ASTNode&) = delete;
		virtual ~ASTNode() = default;
		
		void analyse(SemanticContext& ctx, AnalysisPass pass);

		virtual ExecutionFlow execute(Runtime& runtime) const = 0;
		UpflowInfo upflow(SemanticContext& ctx);
		UpflowInfo upflow() const;
		const ScriptSegment& segment() const;

	protected:
		virtual void impl_analyse(SemanticContext& ctx, AnalysisPass pass) = 0;
		virtual UpflowInfo impl_upflow(SemanticContext& ctx);
		virtual ScriptSegment impl_segment() const = 0;

	public:
		EvalContext eval_context(Runtime& runtime) const;
	};

	class Block : public ASTNode
	{
		std::vector<ASTNode*> _children;

	public:
		virtual ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		ExecutionFlow execute_subnodes(Runtime& runtime) const;

		virtual void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		SemanticContext::LocalScope enter_scope(SemanticContext& ctx);
		virtual void analyse_subnodes(SemanticContext& ctx, AnalysisPass pass);
		virtual UpflowInfo impl_upflow(SemanticContext& ctx) override;

	protected:
		virtual bool isolated() const = 0;

	public:
		void append(ASTNode& child);
	};

	class IsolationBlock : public Block
	{
		std::optional<UpflowInfo> _block_upflow;

	protected:
		void analyse_subnodes(SemanticContext& ctx, AnalysisPass pass) override;

		bool isolated() const override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
		UpflowInfo block_upflow(SemanticContext& ctx);
		UpflowInfo block_upflow() const;
	};

	class ASTRoot : public IsolationBlock
	{
		Token _start_token;

	public:
		ASTRoot(Token&& start_token);

		void analyse_tree(SemanticContext& ctx);

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
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

	protected:
		mutable bool _validated = false;

	public:
		ExecutionFlow execute(Runtime& runtime) const override;

		DataType evaltype(SemanticContext& ctx) const;
		DataType evaltype() const;

		virtual Variable evaluate(Runtime& runtime) const = 0;
		virtual bool imperative() const;

	protected:
		virtual DataType impl_evaltype(SemanticContext& ctx) const = 0;
	};

	class VariableDeclaration : public ASTNode
	{
		bool _global;
		Token _identifier;
		Expression& _expression;

	public:
		VariableDeclaration(bool global, Token&& identifier, Expression& expression);

		ExecutionFlow execute(Runtime& runtime) const override;
		
	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
		
	public:
		bool is_global() const;
	};

	class LiteralExpression : public Expression
	{
		Token _literal;

	public:
		LiteralExpression(Token&& literal);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class ListExpression : public Expression
	{
		mutable std::optional<DataType> _underlying;
		std::vector<Expression*> _elements;
		Token _lbracket_token;
		Token _rbracket_token;

	public:
		ListExpression(Token&& lbracket_token, Token&& rbracket_token, std::vector<Expression*>&& elements, std::optional<DataType>&& underlying);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class BinaryExpression : public Expression
	{
		Token _op;
		Expression& _left;
		Expression& _right;

	public:
		BinaryExpression(Token&& op, Expression& left, Expression& right);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		bool imperative() const override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		BinaryOperator op() const;
	};

	class MemberAccessExpression : public Expression
	{
		Expression& _object;
		Token _member_name;
		bool _callable = false;
		mutable std::optional<MemberSignature> _member;

	public:
		MemberAccessExpression(Expression& object, Token&& member);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		const MemberSignature& member(SemanticContext& ctx) const;
		const MemberSignature& member() const;
		const Expression& object() const;

		void set_callable(bool callable);
	};

	class PrefixExpression : public Expression
	{
		Token _op;
		Expression& _expr;

	public:
		PrefixExpression(Token&& op, Expression& expr);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		PrefixOperator op() const;
	};

	class AsExpression : public Expression
	{
		Expression& _expr;
		FullTypeKeyword _type;

	public:
		AsExpression(Expression& expr, FullTypeKeyword&& type);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class SubscriptExpression : public Expression
	{
		Expression& _container;
		Expression& _subscript;
		Token _closing_bracket;
		mutable std::optional<MemberSignature> _member;

	public:
		SubscriptExpression(Expression& container, Expression& subscript, Token&& closing_bracket);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		const MemberSignature& member(SemanticContext& ctx) const;
		const MemberSignature& member() const;
	};

	class VariableExpression : public Expression
	{
		Token _identifier;

	public:
		VariableExpression(Token&& identifier);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class GlobalMatchesExpression : public Expression
	{
		Token _symbol_token;

	public:
		GlobalMatchesExpression(Token&& symbol_token);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class PatternSymbolExpression : public Expression
	{
		Token _symbol_token;
		BuiltinSymbol _builtin_symbol;

	public:
		PatternSymbolExpression(Token&& symbol_token, BuiltinSymbol builtin_symbol);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class FunctionCallExpression : public Expression
	{
		Token _identifier;
		std::vector<Expression*> _args;
		Token _closing_paren;

	public:
		FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args, Token&& closing_paren);

		Variable evaluate(Runtime& runtime) const override;
		bool imperative() const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		std::vector<DataType> arg_types(SemanticContext& ctx) const;
		std::vector<DataType> arg_types() const;
	};

	class MethodCallExpression : public Expression
	{
		MemberAccessExpression& _member;
		std::vector<Expression*> _args;
		Token _closing_paren;

	public:
		MethodCallExpression(MemberAccessExpression& member, std::vector<Expression*>&& args, Token&& closing_paren);

		Variable evaluate(Runtime& runtime) const override;
		bool imperative() const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class FunctionDefinition : public IsolationBlock
	{
		Token _fn_token;
		Token _identifier;
		std::vector<std::pair<FullTypeKeyword, Token>> _arglist;
		std::optional<FullTypeKeyword> _return_type;

	public:
		FunctionDefinition(Token&& fn_token, Token&& identifier, std::vector<std::pair<FullTypeKeyword, Token>>&& arglist, std::optional<FullTypeKeyword>&& return_type);

		ExecutionFlow execute(Runtime& runtime) const override;
		InvokeResult invoke(Runtime& runtime, std::vector<Variable>&& arguments) const;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
		ScriptSegment impl_segment() const override;

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

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
		ScriptSegment impl_segment() const override;
		
	public:
		DataType evaltype(SemanticContext& ctx) const;
	};

	class IfConditionalBlock;
	
	class IfFallback
	{
		friend IfConditionalBlock;

	protected:
		virtual void fallback_analyse(SemanticContext& ctx, AnalysisPass pass) = 0;
		virtual ExecutionFlow fallback_execute(Runtime& runtime) const = 0;
		virtual UpflowInfo fallback_upflow(SemanticContext& ctx) = 0;
	};
	
	class IfConditionalBlock : public Block
	{
		Expression& _condition;
		std::optional<UpflowInfo> _block_upflow;

	protected:
		IfFallback* _fallback = nullptr;

		IfConditionalBlock(Expression& condition);

		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;

	public:
		void set_fallback(IfFallback* fallback);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		UpflowInfo impl_upflow(SemanticContext& ctx) override;

	private:
		UpflowInfo block_upflow(SemanticContext& ctx);
		UpflowInfo block_upflow() const;
	};

	class IfStatement : public IfConditionalBlock
	{
		Token _if_token;

	public:
		IfStatement(Token&& if_token, Expression& condition);

	protected:
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
		ScriptSegment impl_segment() const override;
		bool isolated() const override;
	};

	class ElifStatement : public IfConditionalBlock, public IfFallback
	{
		Token _elif_token;

	public:
		ElifStatement(Token&& elif_token, Expression& condition);

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
		ScriptSegment impl_segment() const override;
		bool isolated() const override;

		void fallback_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ExecutionFlow fallback_execute(Runtime& runtime) const override;
		UpflowInfo fallback_upflow(SemanticContext& ctx) override;
	};

	class ElseStatement : public IfFallback, public Block
	{
		Token _else_token;

	public:
		ElseStatement(Token&& else_token);

	protected:
		ScriptSegment impl_segment() const override;
		bool isolated() const override;

		void fallback_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ExecutionFlow fallback_execute(Runtime& runtime) const override;
		UpflowInfo fallback_upflow(SemanticContext& ctx) override;
	};

	class Loop : public Block
	{
		Token _loop_token;
		std::optional<UpflowInfo> _block_upflow;

	public:
		Loop(Token&& loop_token);

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		void analyse_subnodes(SemanticContext& ctx, AnalysisPass pass) override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
		ScriptSegment impl_segment() const override;

	private:
		UpflowInfo block_upflow(SemanticContext& ctx);
	};

	class WhileLoop : public Loop
	{
		Expression& _condition;

	public:
		WhileLoop(Token&& loop_token, Expression& condition);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		bool isolated() const override;
	};

	class ForLoop : public Loop
	{
		Token _iterator;
		Expression& _iterable;

	public:
		ForLoop(Token&& loop_token, Token&& iterator, Expression& iterable);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		bool isolated() const override;
	};

	class BreakStatement : public ASTNode
	{
		Token _break_token;
		const Loop* _backloop = nullptr;

	public:
		BreakStatement(Token&& break_token);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
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

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		UpflowInfo impl_upflow(SemanticContext& ctx) override;
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

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
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

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class DeletePattern : public ASTNode
	{
		Token _delete_token;
		Token _identifier;

	public:
		DeletePattern(Token&& delete_token, Token&& identifier);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class PatternDeclaration : public ASTNode
	{
		Token _pattern_token;
		Token _identifier;

	public:
		PatternDeclaration(Token&& pattern_token, Token&& identifier);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class RepeatOperation : public Expression
	{
		Expression& _expression;
		Expression& _range;

	public:
		RepeatOperation(Expression& expression, Expression& range);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class SimpleRepeatOperation : public Expression
	{
		Expression& _expression;
		Token _op;

	public:
		SimpleRepeatOperation(Expression& expression, Token&& op);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;

	public:
		PatternSimpleRepeatOperator op() const;
	};

	class PatternBackRef : public Expression
	{
		Token _ref_token;
		Token _identifier;

	public:
		PatternBackRef(Token&& ref_token, Token&& identifier);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class PatternLazy : public Expression
	{
		Token _lazy_token;
		Expression& _expression;

	public:
		PatternLazy(Token&& lazy_token, Expression& expression);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class PatternGreedy : public Expression
	{
		Token _greedy_token;
		Expression& _expression;

	public:
		PatternGreedy(Token&& greedy_token, Expression& expression);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class PatternCapture : public Expression
	{
		Token _capture_token;
		Token _identifier;
		Expression& _expression;

	public:
		PatternCapture(Token&& capture_token, Token&& identifier, Expression& expression);

		Variable evaluate(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		DataType impl_evaltype(SemanticContext& ctx) const override;
		ScriptSegment impl_segment() const override;
	};

	class AppendStatement : public ASTNode
	{
		Token _append_token;
		Expression& _expression;

	public:
		AppendStatement(Token&& append_token, Expression& expression);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class FindStatement : public ASTNode
	{
		Token _find_token;
		Expression& _pattern;
		bool _findall;

	public:
		FindStatement(Token&& find_token, Expression& pattern, bool findall);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class FilterStatement : public ASTNode
	{
		Token _filter_token;
		Token _identifier;

	public:
		FilterStatement(Token&& filter_token, Token&& identifier);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class ReplaceStatement : public ASTNode
	{
		Token _replace_token;
		Expression& _match;
		Expression& _string;

	public:
		ReplaceStatement(Token&& replace_token, Expression& match, Expression& string);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class ApplyStatement : public ASTNode
	{
		Token _apply_token;
		Token _identifier;

	public:
		ApplyStatement(Token&& apply_token, Token&& identifier);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class ScopeStatement : public ASTNode
	{
		Token _scope_token;
		Token _symbol_token;
		BuiltinSymbol _specifier;
		Expression* _count;

	public:
		ScopeStatement(Token&& scope_token, Token&& symbol_token, BuiltinSymbol specifier, Expression* count);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;

	private:
		Scope scope(Runtime& runtime) const;
	};

	class PagePush : public ASTNode
	{
		Token _page_token;
		Expression& _page;

	public:
		PagePush(Token&& page_token, Expression& page);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class PagePop : public ASTNode
	{
		Token _page_token;

	public:
		PagePop(Token&& page_token);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};

	class PageClearStack : public ASTNode
	{
		Token _page_token;

	public:
		PageClearStack(Token&& page_token);

		ExecutionFlow execute(Runtime& runtime) const override;

	protected:
		void impl_analyse(SemanticContext& ctx, AnalysisPass pass) override;
		ScriptSegment impl_segment() const override;
	};
}
