#include "ast.h"

#include "types/processing.h"
#include "types/iterator.h"
#include "constants.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	namespace errors
	{
		static const char* DOES_NOT_RESOLVE = "expression does not resolve to";
	}

	static DataType assert_implicitly_casts(SemanticContext& ctx, const Expression& expr, DataType to)
	{
		if (expr.evaltype(ctx).can_cast_implicit(to))
			return to;

		std::stringstream ss;
		ss << errors::DOES_NOT_RESOLVE << to;
		throw LxError::segment_error(expr.segment(), ErrorType::Internal, ss.str());
	}

	static void validate_implicitly_casts(SemanticContext& ctx, const Expression& expr, DataType to)
	{
		if (!expr.evaltype(ctx).can_cast_implicit(to))
		{
			std::stringstream ss;
			ss << errors::DOES_NOT_RESOLVE << to;
			ctx.add_semantic_error(expr.segment(), ss.str());
		}
	}

	FullTypeKeyword::FullTypeKeyword(const Token& simple, const std::vector<Token>& underlying)
		: _simple(simple.keyword()), _segment(underlying.empty() ? simple.segment : simple.segment.combined_right(underlying.back().segment))
	{
		for (const Token& token : underlying)
			_underlying.push_back(token.keyword());
	}

	DataType FullTypeKeyword::type() const
	{
		return data_type(_simple, _underlying);
	}

	const ScriptSegment& FullTypeKeyword::segment() const
	{
		return _segment;
	}

	void UpflowInfo::merge_loop_control(const UpflowInfo& other)
	{
		may_break |= other.may_break;
		may_continue |= other.may_continue;
		breaks.insert(breaks.end(), other.breaks.begin(), other.breaks.end());
		continues.insert(continues.end(), other.continues.begin(), other.continues.end());
	}

	void ASTNode::analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		try
		{
			impl_analyse(ctx, pass);
		}
		catch (const LxError& e)
		{
			ctx.errors().push_back(e);
			// TODO v0.2 for internal errors, log in optional debug log and just print "Internal error - see debug log" in regular log
		}
	}

	UpflowInfo ASTNode::upflow(SemanticContext& ctx)
	{
		if (!_upflow)
			_upflow = impl_upflow(ctx);
		return *_upflow;
	}

	UpflowInfo ASTNode::upflow() const
	{
		if (!_upflow)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": upflow not set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			return *_upflow;
	}

	UpflowInfo ASTNode::impl_upflow(SemanticContext& ctx)
	{
		return {};
	}

	EvalContext ASTNode::eval_context(Runtime& runtime) const
	{
		return EvalContext{ .runtime = runtime, .segment = &segment() };
	}

	const ScriptSegment& ASTNode::segment() const
	{
		if (!_segment)
			_segment = impl_segment();
		return *_segment;
	}

	ASTNode& AbstractSyntaxTree::impl_add(std::unique_ptr<ASTNode>&& node)
	{
		ASTNode* ref = node.get();
		_nodes.push_back(std::move(node));
		return *ref;
	}

	AbstractSyntaxTree::AbstractSyntaxTree(Token&& start_token)
		: _root(std::move(start_token))
	{
	}

	const ASTRoot& AbstractSyntaxTree::root() const
	{
		return _root;
	}

	ASTRoot& AbstractSyntaxTree::root()
	{
		return _root;
	}

	void Block::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		auto scope = enter_scope(ctx);
		analyse_subnodes(ctx, pass);
	}

	ExecutionFlow Block::execute(Runtime& runtime) const
	{
		Runtime::LocalScope local_scope(runtime, isolated());
		return execute_subnodes(runtime);
	}

	ExecutionFlow Block::execute_subnodes(Runtime& runtime) const
	{
		ExecutionFlow flow{};
		for (const ASTNode* node : _children)
		{
			auto result = node->execute(runtime);
			if (result.type != FlowType::Normal)
			{
				flow = std::move(result);
				break;
			}
		}
		return flow;
	}

	SemanticContext::LocalScope Block::enter_scope(SemanticContext& ctx)
	{
		return SemanticContext::LocalScope(ctx, isolated());
	}

	void Block::analyse_subnodes(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass != AnalysisPass::VarConsistencySetup || !ctx.in_local_scope())
			for (ASTNode* node : _children)
				node->analyse(ctx, pass);
	}

	UpflowInfo Block::impl_upflow(SemanticContext& ctx)
	{
		UpflowInfo info;
		bool livecode = true;
		std::vector<ScriptSegment> warning_segments;

		for (ASTNode* node : _children)
		{
			auto subinfo = node->upflow(ctx);
			info.dead_returns.insert(info.dead_returns.end(), subinfo.dead_returns.begin(), subinfo.dead_returns.end());
			if (livecode)
			{
				info.live_returns.insert(info.live_returns.end(), subinfo.live_returns.begin(), subinfo.live_returns.end());
				info.always_returns |= subinfo.always_returns;
				info.merge_loop_control(subinfo);

				livecode = !info.always_returns;
			}
			else
			{
				info.dead_returns.insert(info.dead_returns.end(), subinfo.live_returns.begin(), subinfo.live_returns.end());
				warning_segments.push_back(node->segment());
			}
		}

		if (!warning_segments.empty())
			ctx.warnings().push_back(LxWarning::batch_warning(warning_segments, ErrorType::Semantic, "unreachable code"));

		return info;
	}

	void Block::append(ASTNode& child)
	{
		_children.push_back(&child);
	}

	void IsolationBlock::analyse_subnodes(SemanticContext& ctx, AnalysisPass pass)
	{
		Block::analyse_subnodes(ctx, pass);

		if (pass == AnalysisPass::Validation)
		{
			auto flow = block_upflow(ctx);

			for (BreakStatement* b : flow.breaks)
				ctx.add_semantic_error(b->segment(), "no outer loop to break out of");

			for (ContinueStatement* c : flow.continues)
				ctx.add_semantic_error(c->segment(), "no outer loop to continue to");
		}
	}

	bool IsolationBlock::isolated() const
	{
		return true;
	}

	UpflowInfo IsolationBlock::impl_upflow(SemanticContext& ctx)
	{
		return {};
	}

	UpflowInfo IsolationBlock::block_upflow(SemanticContext& ctx)
	{
		if (!_block_upflow)
			_block_upflow = Block::impl_upflow(ctx);
		return *_block_upflow;
	}

	UpflowInfo IsolationBlock::block_upflow() const
	{
		if (!_block_upflow)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": block upflow not set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			return *_block_upflow;
	}

	ASTRoot::ASTRoot(Token&& start_token)
		: _start_token(std::move(start_token))
	{
	}

	void ASTRoot::analyse_tree(SemanticContext& ctx)
	{
		analyse(ctx, AnalysisPass::Registration);
		analyse(ctx, AnalysisPass::Validation);
		analyse(ctx, AnalysisPass::VarConsistencySetup);
	}

	void ASTRoot::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		analyse_subnodes(ctx, pass);
	}

	ScriptSegment ASTRoot::impl_segment() const
	{
		return _start_token.segment;
	}

	ExecutionFlow Expression::execute(Runtime& runtime) const
	{
		if (imperative())
			evaluate(runtime);
		return {};
	}

	DataType Expression::evaltype(SemanticContext& ctx) const
	{
		if (!_validated)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": node not validated";
			throw LxError(ErrorType::Internal, ss.str());
		}

		if (!_evaltype)
			_evaltype = impl_evaltype(ctx);

		return *_evaltype;
	}

	DataType Expression::evaltype() const
	{
		if (!_evaltype)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": evaltype not set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			return *_evaltype;
	}

	bool Expression::imperative() const
	{
		return false;
	}

	VariableDeclaration::VariableDeclaration(bool global, Token&& identifier, Expression& expression)
		: _global(global), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableDeclaration::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (_global && ctx.in_local_scope())
				ctx.add_semantic_error(_identifier, "cannot declare global variable inside a scope");
			else if (auto ln = ctx.identifier_first_decl_line_number(_identifier.lexeme, _global ? Namespace::Unknown : Namespace::Local))
				ctx.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(*ln));
			else
			{
				_expression.analyse(ctx, pass);
				ctx.register_variable(_identifier.lexeme, _expression.evaltype(ctx), _identifier.segment.start_line, _global ? Namespace::Global : Namespace::Local);
			}
		}

		if (pass == AnalysisPass::VarConsistencySetup || pass == AnalysisPass::VarConsistencyExec)
		{
			if (_global)
				ctx.var_consistency_test().declare_global(_identifier.lexeme);
			else
				ctx.var_consistency_test().declare_local(_identifier.lexeme);
		}
	}

	ExecutionFlow VariableDeclaration::execute(Runtime& runtime) const
	{
		runtime.register_variable(_identifier.lexeme, _expression.evaluate(runtime).consume(), _global ? Namespace::Global : Namespace::Local);
		return {};
	}

	ScriptSegment VariableDeclaration::impl_segment() const
	{
		return _identifier.segment.combined_right(_expression.segment());
	}

	bool VariableDeclaration::is_global() const
	{
		return _global;
	}

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void LiteralExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			evaltype(ctx);
		}
	}
	
	Variable LiteralExpression::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(DataPoint::make_from_literal(eval_context(runtime), literal_type(_literal.type), _literal.resolved()));
	}

	DataType LiteralExpression::impl_evaltype(SemanticContext& ctx) const
	{
		return literal_type(_literal.type);
	}

	ScriptSegment LiteralExpression::impl_segment() const
	{
		return _literal.segment;
	}

	ListExpression::ListExpression(Token&& lbracket_token, Token&& rbracket_token, std::vector<Expression*>&& elements, std::optional<DataType>&& underlying)
		: _lbracket_token(std::move(lbracket_token)), _rbracket_token(std::move(rbracket_token)), _elements(std::move(elements)), _underlying(std::move(underlying))
	{
	}
	
	void ListExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			for (Expression* expr : _elements)
				expr->analyse(ctx, pass);
			evaltype(ctx);
		}
	}

	Variable ListExpression::evaluate(Runtime& runtime) const
	{
		std::vector<Variable> elements;
		for (const Expression* expr : _elements)
			elements.push_back(runtime.unbound_variable(expr->evaluate(runtime).consume()));

		if (!elements.empty())
			return runtime.unbound_variable(List(eval_context(runtime), std::move(elements)));
		else
			return runtime.unbound_variable(List(eval_context(runtime), _underlying.value()));
	}

	DataType ListExpression::impl_evaltype(SemanticContext& ctx) const
	{
		if (!_underlying)
		{
			if (_elements.empty())
				throw LxError::segment_error(segment(), ErrorType::Internal, "list expression with unresolved type can not analyse type with no elements");

			std::vector<LxError> errors;
			DataType underlying = _elements[0]->evaltype(ctx);
			for (size_t i = 1; i < _elements.size(); ++i)
			{
				if (_elements[i]->evaltype() != underlying)
				{
					std::stringstream ss;
					ss << "first element in list has type " << *_underlying << ", but element resolved to " << _elements[i]->evaltype();
					errors.push_back(LxError::segment_error(_elements[i]->segment(), ErrorType::Runtime, ss.str()));
				}
			}
			_underlying = std::move(underlying);
		}

		return DataType::List(*_underlying);
	}

	ScriptSegment ListExpression::impl_segment() const
	{
		return _lbracket_token.segment.combined_right(_rbracket_token.segment);
	}

	BinaryExpression::BinaryExpression(Token&& op, Expression& left, Expression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void BinaryExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_left.analyse(ctx, pass);
			_right.analyse(ctx, pass);
			evaltype(ctx);
		}
	}

	Variable BinaryExpression::evaluate(Runtime& runtime) const
	{
		return operate(eval_context(runtime), op(), _left.evaluate(runtime), _right.evaluate(runtime));
	}

	bool BinaryExpression::imperative() const
	{
		return is_imperative(op());
	}

	DataType BinaryExpression::impl_evaltype(SemanticContext& ctx) const
	{
		if (auto type = lx::evaltype(op(), _left.evaltype(ctx), _right.evaltype(ctx)))
			return *type;
		else
		{
			std::stringstream ss;
			ss << ": operator not defined for types " << _left.evaltype(ctx) << " and " << _right.evaltype(ctx);
			ctx.add_semantic_error(_op, ss.str());
			return DataType::Void();
		}
	}

	ScriptSegment BinaryExpression::impl_segment() const
	{
		return _left.segment().combined_right(_right.segment());
	}

	BinaryOperator BinaryExpression::op() const
	{
		return binary_operator(_op);
	}

	MemberAccessExpression::MemberAccessExpression(Expression& object, Token&& member)
		: _object(object), _member_name(std::move(member))
	{
	}

	void MemberAccessExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_object.analyse(ctx, pass);
			member(ctx);
		}
	}

	Variable MemberAccessExpression::evaluate(Runtime& runtime) const
	{
		const MemberSignature& m = member();
		if (m.is_data())
			return _object.evaluate(runtime).data_member(eval_context(runtime), m.identifier());
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member \"" << m.identifier() << "\" is not data layout";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	DataType MemberAccessExpression::impl_evaltype(SemanticContext& ctx) const
	{
		return member(ctx).data_type();
	}

	ScriptSegment MemberAccessExpression::impl_segment() const
	{
		return _object.segment().combined_right(_member_name.segment);
	}

	const MemberSignature& MemberAccessExpression::member(SemanticContext& ctx) const
	{
		if (_member)
			return *_member;

		MemberSignature member;
		if (_object.evaltype(ctx).member(_member_name.lexeme, member))
		{
			if (_callable ? member.is_method() : member.is_data())
			{
				_member = std::move(member);
				return *_member;
			}
		}

		std::stringstream ss;
		ss << "member " << _member_name.lexeme << " does not exist for type " << _object.evaltype(ctx);
		throw LxError::segment_error(_member_name.segment, ErrorType::Semantic, ss.str());
	}

	const MemberSignature& MemberAccessExpression::member() const
	{
		if (_member)
			return *_member;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member not set";
			throw LxError::segment_error(segment(), ErrorType::Internal, ss.str());
		}
	}

	const Expression& MemberAccessExpression::object() const
	{
		return _object;
	}

	void MemberAccessExpression::set_callable(bool callable)
	{
		_callable = callable;
	}

	PrefixExpression::PrefixExpression(Token&& op, Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	void PrefixExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_expr.analyse(ctx, pass);
			evaltype(ctx);
		}
	}

	Variable PrefixExpression::evaluate(Runtime& runtime) const
	{
		return operate(eval_context(runtime), op(), _expr.evaluate(runtime));
	}

	DataType PrefixExpression::impl_evaltype(SemanticContext& ctx) const
	{
		if (auto type = lx::evaltype(op(), _expr.evaltype(ctx)))
			return *type;
		else
		{
			std::stringstream ss;
			ss << ": operator not defined for type " << _expr.evaltype(ctx);
			ctx.add_semantic_error(_op, ss.str());
			return DataType::Void();
		}
	}

	ScriptSegment PrefixExpression::impl_segment() const
	{
		return _op.segment.combined_right(_expr.segment());
	}

	PrefixOperator PrefixExpression::op() const
	{
		return prefix_operator(_op);
	}

	AsExpression::AsExpression(Expression& expr, FullTypeKeyword&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_expr.analyse(ctx, pass);
			evaltype(ctx);
		}
	}

	Variable AsExpression::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(_expr.evaluate(runtime).cast(eval_context(runtime), _type.type()));
	}

	DataType AsExpression::impl_evaltype(SemanticContext& ctx) const
	{
		DataType return_type = _type.type();
		if (_expr.evaltype(ctx).can_cast_explicit(return_type))
			return return_type;
		else
		{
			std::stringstream ss;
			ss << "cannot convert from " << _expr.evaltype(ctx) << " to " << return_type;
			ctx.add_semantic_error(_type.segment(), ss.str());
			return DataType::Void();
		}
	}

	ScriptSegment AsExpression::impl_segment() const
	{
		return _expr.segment().combined_right(_type.segment());
	}

	SubscriptExpression::SubscriptExpression(Expression& container, Expression& subscript, Token&& closing_bracket)
		: _container(container), _subscript(subscript), _closing_bracket(std::move(closing_bracket))
	{
	}

	void SubscriptExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_container.analyse(ctx, pass);
			_subscript.analyse(ctx, pass);
			evaltype(ctx);
		}
	}

	Variable SubscriptExpression::evaluate(Runtime& runtime) const
	{
		return _container.evaluate(runtime).invoke_method(eval_context(runtime), constants::SUBSCRIPT_OP, { _subscript.evaluate(runtime) });
	}

	DataType SubscriptExpression::impl_evaltype(SemanticContext& ctx) const
	{
		return member(ctx).return_type({ _subscript.evaltype(ctx) }).value();
	}

	ScriptSegment SubscriptExpression::impl_segment() const
	{
		return _container.segment().combined_right(_closing_bracket.segment);
	}

	const MemberSignature& SubscriptExpression::member(SemanticContext& ctx) const
	{
		if (_member)
			return *_member;

		MemberSignature member;
		if (_container.evaltype(ctx).member(constants::SUBSCRIPT_OP, member))
		{
			if (member.is_method() && member.return_type({ _subscript.evaltype(ctx) }).has_value())
			{
				_member = std::move(member);
				return *_member;
			}
		}

		std::stringstream ss;
		ss << _container.evaltype(ctx) << " does not support [] with index type " << _subscript.evaltype(ctx);
		throw LxError::segment_error(_container.segment(), ErrorType::Semantic, ss.str());
	}

	const MemberSignature& SubscriptExpression::member() const
	{
		if (_member)
			return *_member;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member not set";
			throw LxError::segment_error(segment(), ErrorType::Internal, ss.str());
		}
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void VariableExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (ctx.registered_variable(_identifier.lexeme, Namespace::Unknown).has_value())
				_validated = true;
			else
				ctx.add_semantic_error(_identifier, "variable is not declared in scope");

			if (_validated)
			{
				try
				{
					evaltype(ctx);
				}
				catch (const LxError& e)
				{
					ctx.add_semantic_error(_identifier.segment, e.message());
				}
			}
		}

		if (pass == AnalysisPass::VarConsistencyExec)
			ctx.var_consistency_test().test(ctx, _identifier);
	}

	Variable VariableExpression::evaluate(Runtime& runtime) const
	{
		return runtime.registered_variable(_identifier.lexeme, Namespace::Unknown, segment());
	}

	DataType VariableExpression::impl_evaltype(SemanticContext& ctx) const
	{
		if (auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown))
			return var->type;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable is not registered";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	ScriptSegment VariableExpression::impl_segment() const
	{
		return _identifier.segment;
	}

	GlobalMatchesExpression::GlobalMatchesExpression(Token&& symbol_token)
		: _symbol_token(std::move(symbol_token))
	{
	}

	void GlobalMatchesExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			evaltype(ctx);
		}
	}

	Variable GlobalMatchesExpression::evaluate(Runtime& runtime) const
	{
		return runtime.global_matches_var();
	}

	DataType GlobalMatchesExpression::impl_evaltype(SemanticContext& ctx) const
	{
		return DataType::Matches();
	}

	ScriptSegment GlobalMatchesExpression::impl_segment() const
	{
		return _symbol_token.segment;
	}

	PatternSymbolExpression::PatternSymbolExpression(Token&& symbol_token, BuiltinSymbol builtin_symbol)
		: _symbol_token(std::move(symbol_token)), _builtin_symbol(builtin_symbol)
	{
	}

	void PatternSymbolExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			evaltype(ctx);
		}
	}

	Variable PatternSymbolExpression::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(Pattern::make_from_symbol(_builtin_symbol));
	}

	DataType PatternSymbolExpression::impl_evaltype(SemanticContext& ctx) const
	{
		return DataType::Pattern();
	}

	ScriptSegment PatternSymbolExpression::impl_segment() const
	{
		return _symbol_token.segment;
	}

	FunctionCallExpression::FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args, Token&& closing_paren)
		: _identifier(std::move(identifier)), _args(std::move(args)), _closing_paren(std::move(closing_paren))
	{
	}

	void FunctionCallExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (ctx.registered_function_calls(_identifier.lexeme).empty())
				ctx.add_semantic_error(_identifier, "function not declared");
			else
				_validated = true;

			if (_validated)
			{
				for (Expression* arg : _args)
					arg->analyse(ctx, pass);

				auto argtypes = arg_types(ctx);
				if (!ctx.registered_function(_identifier.lexeme, argtypes))
				{
					std::stringstream ss;
					ss << "no declaration of '" << _identifier.lexeme << "' matches the argument types ";
					print_list(ss, argtypes);
					ctx.add_semantic_error(segment(), ss.str());
				}
			}
		}

		if (_validated)
		{
			if (auto fn = ctx.registered_function(_identifier.lexeme, arg_types(ctx)))
			{
				if (pass == AnalysisPass::VarConsistencySetup)
				{
					ctx.var_consistency_test().see(*fn->decl_node, *this);
					fn->decl_node->analyse(ctx, AnalysisPass::VarConsistencyExec);
					ctx.var_consistency_test().clear_stack();
				}

				if (pass == AnalysisPass::VarConsistencyExec && !ctx.var_consistency_test().seen(*fn->decl_node))
				{
					ctx.var_consistency_test().see(*fn->decl_node, *this);
					fn->decl_node->analyse(ctx, pass);
					ctx.var_consistency_test().exit_scope();
				}
			}
		}
	}

	Variable FunctionCallExpression::evaluate(Runtime& runtime) const
	{
		const FunctionDefinition& fn = runtime.registered_function(_identifier.lexeme, arg_types(), segment());
		std::vector<Variable> arguments;
		for (const Expression* arg : _args)
			arguments.push_back(arg->evaluate(runtime));
		return fn.invoke(runtime, std::move(arguments)).data;
	}

	bool FunctionCallExpression::imperative() const
	{
		return true;
	}

	DataType FunctionCallExpression::impl_evaltype(SemanticContext& ctx) const
	{
		if (auto fn = ctx.registered_function(_identifier.lexeme, arg_types(ctx)))
			return fn->return_type;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function is not registered";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	ScriptSegment FunctionCallExpression::impl_segment() const
	{
		return _identifier.segment.combined_right(_closing_paren.segment);
	}

	std::vector<DataType> FunctionCallExpression::arg_types(SemanticContext& ctx) const
	{
		std::vector<DataType> args;
		for (const Expression* arg : _args)
			args.push_back(arg->evaltype(ctx));
		return args;
	}

	std::vector<DataType> FunctionCallExpression::arg_types() const
	{
		std::vector<DataType> args;
		for (const Expression* arg : _args)
			args.push_back(arg->evaltype());
		return args;
	}

	MethodCallExpression::MethodCallExpression(MemberAccessExpression& member, std::vector<Expression*>&& args, Token&& closing_paren)
		: _member(member), _args(std::move(args)), _closing_paren(std::move(closing_paren))
	{
		_member.set_callable(true);
	}

	void MethodCallExpression::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
		
			_member.analyse(ctx, pass);
			for (Expression* arg : _args)
				arg->analyse(ctx, pass);

			try
			{
				evaltype(ctx);
			}
			catch (const LxError& e)
			{
				ctx.add_semantic_error(segment(), e.message());
			}
		}
	}

	Variable MethodCallExpression::evaluate(Runtime& runtime) const
	{
		const MemberSignature& m = _member.member();
		if (m.is_method())
		{
			std::vector<Variable> args;
			for (const Expression* expr : _args)
				args.push_back(expr->evaluate(runtime));

			return _member.object().evaluate(runtime).invoke_method(eval_context(runtime), m.identifier(), std::move(args));
		}
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member \"" << m.identifier() << "\" is not method";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	bool MethodCallExpression::imperative() const
	{
		return true;
	}

	DataType MethodCallExpression::impl_evaltype(SemanticContext& ctx) const
	{
		const MemberSignature& m = _member.member(ctx);
		if (m.is_method())
		{
			std::vector<DataType> arg_types;
			for (Expression* arg : _args)
				arg_types.push_back(arg->evaltype(ctx));

			if (auto r = m.return_type(arg_types))
				return *r;

			std::stringstream ss;
			ss << "no overloads exist for '" << m.identifier() << "' with arguments ";
			print_list(ss, arg_types);
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
		{
			std::stringstream ss;
			ss << "'" << m.identifier() << "' is not callable";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	ScriptSegment MethodCallExpression::impl_segment() const
	{
		return _member.segment().combined_right(_closing_paren.segment);
	}

	FunctionDefinition::FunctionDefinition(Token&& fn_token, Token&& identifier, std::vector<std::pair<FullTypeKeyword, Token>>&& arglist, std::optional<FullTypeKeyword>&& return_type)
		: _fn_token(std::move(fn_token)), _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	void FunctionDefinition::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Registration)
		{
			if (auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown))
			{
				ctx.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(var->decl_line_number));
				return;
			}
		
			if (auto fn = ctx.registered_function(_identifier.lexeme, arg_types()))
			{
				ctx.add_semantic_error(_identifier, "function with matching argument types already declared on line " + std::to_string(fn->decl_line_number));
				return;
			}

			std::unordered_set<std::string_view> argnames;
			for (const auto& [_, identifier] : _arglist)
			{
				if (argnames.contains(identifier.lexeme))
					ctx.add_semantic_error(identifier, "repeated argument identifier");
				else
					argnames.insert(identifier.lexeme);
			}

			if (argnames.size() != _arglist.size())
				return;

			if (ctx.in_local_scope())
			{
				ctx.add_semantic_error(_identifier, "cannot define function inside local scope");
				return;
			}

			ctx.register_function(*this, _identifier.lexeme, return_type(), arg_types(), _identifier.segment.start_line);
		}
	
		auto scope = enter_scope(ctx);

		if (pass == AnalysisPass::Validation)
		{
			auto argtypes = arg_types();
			for (size_t i = 0; i < _arglist.size(); ++i)
				ctx.register_variable(_arglist[i].second.lexeme, std::move(argtypes[i]), _arglist[i].second.segment.start_line, Namespace::Local);
		}

		IsolationBlock::analyse_subnodes(ctx, pass);

		if (pass == AnalysisPass::Validation)
		{
			auto flow = block_upflow(ctx);

			if (return_type().simple() != SimpleType::Void && !flow.always_returns)
				ctx.add_semantic_error(_identifier, "not all control paths return a value");

			for (const ReturnStatement* r : flow.live_returns)
			{
				if (r->evaltype(ctx) != return_type())
				{
					std::stringstream ss;
					ss << "function should return " << return_type() << " but statement returns " << r->evaltype(ctx);
					ctx.add_semantic_error(r->segment(), ss.str());
				}
			}

			for (const ReturnStatement* r : flow.dead_returns)
			{
				if (r->evaltype(ctx) != return_type())
				{
					std::stringstream ss;
					ss << "function should return " << return_type() << " but (unreachable) statement returns " << r->evaltype(ctx);
					ctx.add_semantic_warning(r->segment(), ss.str());
				}
			}
		}
	}

	ExecutionFlow FunctionDefinition::execute(Runtime& runtime) const
	{
		// NOP
		return {};
	}

	InvokeResult FunctionDefinition::invoke(Runtime& runtime, std::vector<Variable>&& arguments) const
	{
		Runtime::LocalScope local_scope(runtime, isolated());

		for (size_t i = 0; i < _arglist.size(); ++i)
			runtime.register_variable(_arglist[i].second.lexeme, std::move(arguments[i]).consume(), Namespace::Local);

		auto flow = execute_subnodes(runtime);
		return { .data = flow.data ? *flow.data : runtime.unbound_variable(Void()) };
	}

	UpflowInfo FunctionDefinition::impl_upflow(SemanticContext& ctx)
	{
		return {};
	}

	ScriptSegment FunctionDefinition::impl_segment() const
	{
		return _fn_token.segment.combined_right(_identifier.segment);
	}

	std::vector<DataType> FunctionDefinition::arg_types() const
	{
		std::vector<DataType> types;
		for (const auto& arg : _arglist)
			types.push_back(arg.first.type());
		return types;
	}
	
	std::vector<std::string_view> FunctionDefinition::arg_identifiers() const
	{
		std::vector<std::string_view> identifiers;
		for (const auto& arg : _arglist)
			identifiers.push_back(arg.second.lexeme);
		return identifiers;
	}
	
	DataType FunctionDefinition::return_type() const
	{
		return _return_type ? _return_type->type() : DataType::Void();
	}

	ReturnStatement::ReturnStatement(Token&& return_token, Expression* expression)
		: _return_token(std::move(return_token)), _expression(expression)
	{
	}

	void ReturnStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (_expression)
			_expression->analyse(ctx, pass);
	}

	ExecutionFlow ReturnStatement::execute(Runtime& runtime) const
	{
		if (_expression)
			return { .type = FlowType::Return, .data = _expression->evaluate(runtime) };
		else
			return { .type = FlowType::Return, .data = runtime.unbound_variable(Void()) };
	}

	UpflowInfo ReturnStatement::impl_upflow(SemanticContext& ctx)
	{
		return { .always_returns = true, .live_returns = { this } };
	}

	ScriptSegment ReturnStatement::impl_segment() const
	{
		return _expression ? _expression->segment() : _return_token.segment;
	}

	DataType ReturnStatement::evaltype(SemanticContext& ctx) const
	{
		return _expression ? _expression->evaltype(ctx) : DataType::Void();
	}

	IfConditionalBlock::IfConditionalBlock(Expression& condition)
		: _condition(condition)
	{
	}

	void IfConditionalBlock::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		_condition.analyse(ctx, pass);

		if (pass == AnalysisPass::Validation)
			validate_implicitly_casts(ctx, _condition, DataType::Bool());

		Block::impl_analyse(ctx, pass);
		if (_fallback)
			_fallback->fallback_analyse(ctx, pass);
	}

	void IfConditionalBlock::set_fallback(IfFallback* fallback)
	{
		if (_fallback)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": fallback already set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			_fallback = fallback;
	}

	ExecutionFlow IfConditionalBlock::execute(Runtime& runtime) const
	{
		if (_condition.evaluate(runtime).consume_as<Bool>(eval_context(runtime)).value())
			return Block::execute(runtime);
		else if (_fallback)
			return _fallback->fallback_execute(runtime);
		else
			return {};
	}

	UpflowInfo IfConditionalBlock::impl_upflow(SemanticContext& ctx)
	{
		auto info = block_upflow(ctx);
		if (_fallback)
		{
			auto fallback_info = _fallback->fallback_upflow(ctx);
			info.live_returns.insert(info.live_returns.end(), fallback_info.live_returns.begin(), fallback_info.live_returns.end());
			info.dead_returns.insert(info.dead_returns.end(), fallback_info.dead_returns.begin(), fallback_info.dead_returns.end());
			info.always_returns &= fallback_info.always_returns;
			info.merge_loop_control(fallback_info);
		}
		else
			info.always_returns = false;
		return info;
	}

	UpflowInfo IfConditionalBlock::block_upflow(SemanticContext& ctx)
	{
		if (!_block_upflow)
			_block_upflow = Block::impl_upflow(ctx);
		return *_block_upflow;
	}

	UpflowInfo IfConditionalBlock::block_upflow() const
	{
		if (!_block_upflow)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": block upflow not set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			return *_block_upflow;
	}

	IfStatement::IfStatement(Token&& if_token, Expression& condition)
		: IfConditionalBlock(condition), _if_token(std::move(if_token))
	{
	}

	UpflowInfo IfStatement::impl_upflow(SemanticContext& ctx)
	{
		return IfConditionalBlock::impl_upflow(ctx);
	}

	ScriptSegment IfStatement::impl_segment() const
	{
		return _if_token.segment;
	}

	bool IfStatement::isolated() const
	{
		return false;
	}

	ElifStatement::ElifStatement(Token&& elif_token, Expression& condition)
		: IfConditionalBlock(condition), _elif_token(std::move(elif_token))
	{
	}

	void ElifStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		IfConditionalBlock::impl_analyse(ctx, pass);
	}

	UpflowInfo ElifStatement::impl_upflow(SemanticContext& ctx)
	{
		return IfConditionalBlock::impl_upflow(ctx);
	}

	ScriptSegment ElifStatement::impl_segment() const
	{
		return _elif_token.segment;
	}

	bool ElifStatement::isolated() const
	{
		return false;
	}

	void ElifStatement::fallback_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		analyse(ctx, pass);
	}
	
	ExecutionFlow ElifStatement::fallback_execute(Runtime& runtime) const
	{
		return execute(runtime);
	}

	UpflowInfo ElifStatement::fallback_upflow(SemanticContext& ctx)
	{
		return upflow(ctx);
	}

	ElseStatement::ElseStatement(Token&& else_token)
		: _else_token(std::move(else_token))
	{
	}

	ScriptSegment ElseStatement::impl_segment() const
	{
		return _else_token.segment;
	}
	
	bool ElseStatement::isolated() const
	{
		return false;
	}

	void ElseStatement::fallback_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		analyse(ctx, pass);
	}

	ExecutionFlow ElseStatement::fallback_execute(Runtime& runtime) const
	{
		return execute(runtime);
	}

	UpflowInfo ElseStatement::fallback_upflow(SemanticContext& ctx)
	{
		return upflow(ctx);
	}

	Loop::Loop(Token&& loop_token)
		: _loop_token(std::move(loop_token))
	{
	}

	void Loop::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		auto scope = enter_scope(ctx);
		analyse_subnodes(ctx, pass);
	}

	void Loop::analyse_subnodes(SemanticContext& ctx, AnalysisPass pass)
	{
		Block::analyse_subnodes(ctx, pass);

		auto subflow = block_upflow(ctx);

		for (BreakStatement* b : subflow.breaks)
			b->attach_loop(this);

		for (ContinueStatement* c : subflow.continues)
			c->attach_loop(this);
	}

	UpflowInfo Loop::impl_upflow(SemanticContext& ctx)
	{
		auto info = block_upflow(ctx);
		info.always_returns &= !info.may_break;
		info.may_break = false;
		info.may_continue = false;
		info.breaks.clear();
		info.continues.clear();
		return info;
	}

	ScriptSegment Loop::impl_segment() const
	{
		return _loop_token.segment;
	}

	UpflowInfo Loop::block_upflow(SemanticContext& ctx)
	{
		if (!_block_upflow)
			_block_upflow = Block::impl_upflow(ctx);
		return *_block_upflow;
	}

	WhileLoop::WhileLoop(Token&& loop_token, Expression& condition)
		: Loop(std::move(loop_token)), _condition(condition)
	{
	}

	void WhileLoop::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		auto scope = enter_scope(ctx);
		_condition.analyse(ctx, pass);
		if (pass == AnalysisPass::Validation)
			validate_implicitly_casts(ctx, _condition, DataType::Bool());
		Loop::analyse_subnodes(ctx, pass);
	}

	ExecutionFlow WhileLoop::execute(Runtime& runtime) const
	{
		while (_condition.evaluate(runtime).consume_as<Bool>(eval_context(runtime)).value())
		{
			auto flow = Block::execute(runtime);
			if (flow.type == FlowType::Break)
				break;
			else if (flow.type == FlowType::Return)
				return flow;
		}
		return {};
	}

	bool WhileLoop::isolated() const
	{
		return false;
	}

	ForLoop::ForLoop(Token&& loop_token, Token&& iterator, Expression& iterable)
		: Loop(std::move(loop_token)), _iterator(std::move(iterator)), _iterable(iterable)
	{
	}

	void ForLoop::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		auto scope = enter_scope(ctx);

		if (pass == AnalysisPass::Validation)
		{
			_iterable.analyse(ctx, pass);
			if (auto itertype = _iterable.evaltype(ctx).itertype())
				ctx.register_variable(_iterator.lexeme, std::move(*itertype), _iterator.segment.start_line, Namespace::Local);
			else
			{
				std::stringstream ss;
				ss << _iterable.evaltype(ctx) << " is not iterable";
				ctx.add_semantic_error(_iterable.segment(), ss.str());
			}
		}

		Loop::analyse_subnodes(ctx, pass);
	}

	ExecutionFlow ForLoop::execute(Runtime& runtime) const
	{
		try
		{
			auto env = eval_context(runtime);
			Iterator iter(_iterable.evaluate(runtime));
			while (!iter.done(env))
			{
				Runtime::LocalScope local_scope(runtime, isolated());
				runtime.register_variable(_iterator.lexeme, iter.get(env), Namespace::Local);
				auto flow = execute_subnodes(runtime);
				if (flow.type == FlowType::Break)
					break;
				else if (flow.type == FlowType::Return)
					return flow;

				iter.next();
			}
			return {};
		}
		catch (const LxError& e)
		{
			if (e.type() == ErrorType::Runtime)
				throw LxError::segment_error(segment(), ErrorType::Runtime, e.message());
			else
				throw;
		}
	}

	bool ForLoop::isolated() const
	{
		return false;
	}

	BreakStatement::BreakStatement(Token&& break_token)
		: _break_token(std::move(break_token))
	{
	}

	void BreakStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
	}

	ExecutionFlow BreakStatement::execute(Runtime& runtime) const
	{
		return { .type = FlowType::Break };
	}

	UpflowInfo BreakStatement::impl_upflow(SemanticContext& ctx)
	{
		return { .may_break = true, .breaks = { this } };
	}

	ScriptSegment BreakStatement::impl_segment() const
	{
		return _break_token.segment;
	}

	void BreakStatement::attach_loop(const Loop* loop)
	{
		if (_backloop)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": loop is already attached";
			throw LxError(ErrorType::Internal, ss.str());
		}

		_backloop = loop;
	}

	ContinueStatement::ContinueStatement(Token&& continue_token)
		: _continue_token(std::move(continue_token))
	{
	}

	void ContinueStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
	}

	ExecutionFlow ContinueStatement::execute(Runtime& runtime) const
	{
		return { .type = FlowType::Continue };
	}

	UpflowInfo ContinueStatement::impl_upflow(SemanticContext& ctx)
	{
		return { .may_continue = true, .continues = { this } };
	}

	ScriptSegment ContinueStatement::impl_segment() const
	{
		return _continue_token.segment;
	}

	void ContinueStatement::attach_loop(const Loop* loop)
	{
		if (_backloop)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": loop is already attached";
			throw LxError(ErrorType::Internal, ss.str());
		}

		_backloop = loop;
	}

	LogStatement::LogStatement(Token&& log_token, std::vector<Expression*>&& args)
		: _log_token(std::move(log_token)), _args(std::move(args))
	{
	}

	void LogStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		for (Expression* arg : _args)
			arg->analyse(ctx, pass);
	}

	ExecutionFlow LogStatement::execute(Runtime& runtime) const
	{
		for (size_t i = 0; i < _args.size(); ++i)
		{
			Variable var = _args[i]->evaluate(runtime);
			var.ref().print(eval_context(runtime), runtime.log());
			if (i + 1 < _args.size())
				runtime.log() << " "; // TODO v0.2 optional separator symbol argument
		}
		runtime.log() << '\n';
		return {};
	}

	ScriptSegment LogStatement::impl_segment() const
	{
		return _log_token.segment;
	}

	HighlightStatement::HighlightStatement(Token&& highlight_token, bool clear, Expression* highlightable, std::optional<Token>&& color_token, BuiltinSymbol color)
		: _highlight_token(std::move(highlight_token)), _clear(clear), _highlightable(highlightable), _color_token(std::move(color_token)), _color(color)
	{
	}

	void HighlightStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (_color_token && !is_color_symbol(_color))
				ctx.add_semantic_error(*_color_token, "symbol is not a color");

			if (_highlightable)
			{
				_highlightable->analyse(ctx, pass);

				if (!_highlightable->evaltype(ctx).is_highlightable())
				{
					std::stringstream ss;
					ss << _highlightable->evaltype(ctx) << " is not highlightable";
					ctx.add_semantic_error(_highlightable->segment(), ss.str());
				}
			}
		}
	}

	ExecutionFlow HighlightStatement::execute(Runtime& runtime) const
	{
		if (_clear)
			runtime.remove_highlight(Color(_color), _highlightable ? std::make_optional(_highlightable->evaluate(runtime)) : std::nullopt, segment());
		else
			runtime.add_highlight(Color(_color), _highlightable ? std::make_optional(_highlightable->evaluate(runtime)) : std::nullopt, segment());

		return {};
	}

	ScriptSegment HighlightStatement::impl_segment() const
	{
		return _highlight_token.segment;
	}

	DeletePattern::DeletePattern(Token&& delete_token, Token&& identifier)
		: _delete_token(std::move(delete_token)), _identifier(std::move(identifier))
	{
	}

	void DeletePattern::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
	}

	ExecutionFlow DeletePattern::execute(Runtime& runtime) const
	{
		runtime.delete_pattern(_identifier.lexeme);
		return {};
	}

	ScriptSegment DeletePattern::impl_segment() const
	{
		return _delete_token.segment.combined_right(_identifier.segment);
	}

	PatternDeclaration::PatternDeclaration(Token&& pattern_token, Token&& identifier)
		: _pattern_token(std::move(pattern_token)), _identifier(std::move(identifier))
	{
	}

	void PatternDeclaration::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Registration)
		{
			if (auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Global))
			{
				if (var->type.simple() != SimpleType::Pattern)
					ctx.add_semantic_error(_identifier.segment, "global variable with same name already declared on line " + std::to_string(var->decl_line_number));
			}
			else
				ctx.register_variable(_identifier.lexeme, DataType::Pattern(), _identifier.segment.start_line, Namespace::Global);
		}
	}

	ExecutionFlow PatternDeclaration::execute(Runtime& runtime) const
	{
		runtime.declare_pattern(_identifier.lexeme);
		return {};
	}

	ScriptSegment PatternDeclaration::impl_segment() const
	{
		return _pattern_token.segment.combined_right(_identifier.segment);
	}

	RepeatOperation::RepeatOperation(Expression& expression, Expression& range)
		: _expression(expression), _range(range)
	{
	}

	void RepeatOperation::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;

			_expression.analyse(ctx, pass);
			_range.analyse(ctx, pass);
			validate_implicitly_casts(ctx, _range, DataType::IRange());

			try
			{
				evaltype(ctx);
			}
			catch (const LxError& e)
			{
				ctx.add_semantic_error(_expression.segment(), e.message());
			}
		}
	}

	Variable RepeatOperation::evaluate(Runtime& runtime) const
	{
		auto env = eval_context(runtime);
		return runtime.unbound_variable(Pattern::make_repeat(_expression.evaluate(runtime).consume_as<Pattern>(env), _range.evaluate(runtime).consume_as<IRange>(env)));
	}

	DataType RepeatOperation::impl_evaltype(SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern());
	}
	
	ScriptSegment RepeatOperation::impl_segment() const
	{
		return _expression.segment().combined_right(_range.segment());
	}

	SimpleRepeatOperation::SimpleRepeatOperation(Expression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}

	void SimpleRepeatOperation::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		_validated = true;
		_expression.analyse(ctx, pass);

		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(_expression.segment(), e.message());
		}
	}

	Variable SimpleRepeatOperation::evaluate(Runtime& runtime) const
	{
		return operate(eval_context(runtime), op(), _expression.evaluate(runtime));
	}

	DataType SimpleRepeatOperation::impl_evaltype(SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern());
	}

	ScriptSegment SimpleRepeatOperation::impl_segment() const
	{
		return _expression.segment().combined_right(_op.segment);
	}

	PatternSimpleRepeatOperator SimpleRepeatOperation::op() const
	{
		return pattern_simple_repeat_operator(_op.type);
	}

	PatternBackRef::PatternBackRef(Token&& ref_token, Token&& identifier)
		: _ref_token(std::move(ref_token)), _identifier(std::move(identifier))
	{
	}

	void PatternBackRef::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
			_validated = true;
	}

	Variable PatternBackRef::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(Pattern::make_backref(runtime.capture_id(_identifier.lexeme)));
	}

	DataType PatternBackRef::impl_evaltype(SemanticContext& ctx) const
	{
		return DataType::Pattern();
	}

	ScriptSegment PatternBackRef::impl_segment() const
	{
		return _ref_token.segment.combined_right(_identifier.segment);
	}

	PatternLazy::PatternLazy(Token&& lazy_token, Expression& expression)
		: _lazy_token(std::move(lazy_token)), _expression(expression)
	{
	}

	void PatternLazy::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_expression.analyse(ctx, pass);

			try
			{
				evaltype(ctx);
			}
			catch (const LxError& e)
			{
				ctx.add_semantic_error(_expression.segment(), e.message());
			}
		}
	}

	Variable PatternLazy::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(Pattern::make_lazy(_expression.evaluate(runtime).consume_as<Pattern>(eval_context(runtime))));
	}

	DataType PatternLazy::impl_evaltype(SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern());
	}

	ScriptSegment PatternLazy::impl_segment() const
	{
		return _lazy_token.segment.combined_right(_expression.segment());
	}

	PatternGreedy::PatternGreedy(Token&& greedy_token, Expression& expression)
		: _greedy_token(std::move(greedy_token)), _expression(expression)
	{
	}

	void PatternGreedy::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_expression.analyse(ctx, pass);

			try
			{
				evaltype(ctx);
			}
			catch (const LxError& e)
			{
				ctx.add_semantic_error(_expression.segment(), e.message());
			}
		}
	}

	Variable PatternGreedy::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(Pattern::make_greedy(_expression.evaluate(runtime).consume_as<Pattern>(eval_context(runtime))));
	}

	DataType PatternGreedy::impl_evaltype(SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern());
	}

	ScriptSegment PatternGreedy::impl_segment() const
	{
		return _greedy_token.segment.combined_right(_expression.segment());
	}

	PatternCapture::PatternCapture(Token&& capture_token, Token&& identifier, Expression& expression)
		: _capture_token(std::move(capture_token)), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void PatternCapture::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_validated = true;
			_expression.analyse(ctx, pass);

			try
			{
				evaltype(ctx);
			}
			catch (const LxError& e)
			{
				ctx.add_semantic_error(_expression.segment(), e.message());
			}
		}
	}

	Variable PatternCapture::evaluate(Runtime& runtime) const
	{
		return runtime.unbound_variable(Pattern::make_capture(
			_expression.evaluate(runtime).consume_as<Pattern>(eval_context(runtime)), runtime.capture_id(_identifier.lexeme)));
	}

	DataType PatternCapture::impl_evaltype(SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern());
	}

	ScriptSegment PatternCapture::impl_segment() const
	{
		return _capture_token.segment.combined_right(_expression.segment());
	}

	AppendStatement::AppendStatement(Token&& append_token, Expression& expression)
		: _append_token(std::move(append_token)), _expression(expression)
	{
	}

	void AppendStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_expression.analyse(ctx, pass);
			validate_implicitly_casts(ctx, _expression, DataType::Pattern());
		}
	}

	ExecutionFlow AppendStatement::execute(Runtime& runtime) const
	{
		runtime.focused_pattern(segment()).ref().get<Pattern>().append(_expression.evaluate(runtime).consume_as<Pattern>(eval_context(runtime)));
		return {};
	}

	ScriptSegment AppendStatement::impl_segment() const
	{
		return _append_token.segment;
	}

	FindStatement::FindStatement(Token&& find_token, Expression& pattern, bool findall)
		: _find_token(std::move(find_token)), _pattern(pattern), _findall(findall)
	{
	}

	void FindStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_pattern.analyse(ctx, pass);
			validate_implicitly_casts(ctx, _pattern, DataType::Pattern());
		}
	}

	ExecutionFlow FindStatement::execute(Runtime& runtime) const
	{
		if (_findall)
			runtime.find_all(segment());
		else
			runtime.search(segment());
		return {};
	}

	ScriptSegment FindStatement::impl_segment() const
	{
		return _find_token.segment.combined_right(_pattern.segment());
	}

	FilterStatement::FilterStatement(Token&& filter_token, Token&& identifier)
		: _filter_token(std::move(filter_token)), _identifier(std::move(identifier))
	{
	}

	void FilterStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (auto fn = ctx.registered_function(_identifier.lexeme, { DataType::Match() }))
			{
				if (fn->return_type.simple() != SimpleType::Bool)
				{
					std::stringstream ss;
					ss << "function on line " << fn->decl_line_number << " is not a match predicate: expected signature (" << DataType::Match() << ") -> " << DataType::Bool();
					ctx.add_semantic_error(_identifier.segment, ss.str());
				}
			}
			else
			{
				std::stringstream ss;
				ss << "no matching predicate function: expected signature (" << DataType::Match() << ") -> " << DataType::Bool();
				ctx.add_semantic_error(_identifier.segment, ss.str());
			}
		}
	}

	ExecutionFlow FilterStatement::execute(Runtime& runtime) const
	{
		const FunctionDefinition& fn = runtime.registered_function(_identifier.lexeme, { DataType::Match() }, segment());

		auto env = eval_context(runtime);
		Matches new_matches;
		Iterator iter(runtime.global_matches_var());
		while (!iter.done(env))
		{
			Variable match = runtime.unbound_variable(iter.get(env));
			if (fn.invoke(runtime, { match }).data.consume_as<Bool>(env).value())
				new_matches.push_back(env, std::move(match));
			iter.next();
		}

		return {};
	}

	ScriptSegment FilterStatement::impl_segment() const
	{
		return _filter_token.segment;
	}

	ReplaceStatement::ReplaceStatement(Token&& replace_token, Expression& match, Expression& string)
		: _replace_token(std::move(replace_token)), _match(match), _string(string)
	{
	}

	void ReplaceStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_match.analyse(ctx, pass);
			_string.analyse(ctx, pass);

			validate_implicitly_casts(ctx, _match, DataType::Match());
			validate_implicitly_casts(ctx, _string, DataType::String());
		}
	}

	ExecutionFlow ReplaceStatement::execute(Runtime& runtime) const
	{
		// TODO
		return {};
	}

	ScriptSegment ReplaceStatement::impl_segment() const
	{
		return _replace_token.segment;
	}
	
	ApplyStatement::ApplyStatement(Token&& apply_token, Token&& identifier)
		: _apply_token(std::move(apply_token)), _identifier(std::move(identifier))
	{
	}

	void ApplyStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (auto fn = ctx.registered_function(_identifier.lexeme, { DataType::Match() }))
			{
				if (fn->return_type != DataType::String())
				{
					std::stringstream ss;
					ss << "found function on line " << fn->decl_line_number << ": expected signature (" << DataType::Match() << ") -> " << DataType::String();
					ctx.add_semantic_error(_identifier.segment, ss.str());
				}
			}
			else
			{
				std::stringstream ss;
				ss << "no matching function: expected signature (" << DataType::Match() << ") -> " << DataType::String();
				ctx.add_semantic_error(_identifier.segment, ss.str());
			}
		}
	}

	ExecutionFlow ApplyStatement::execute(Runtime& runtime) const
	{
		// TODO
		return {};
	}

	ScriptSegment ApplyStatement::impl_segment() const
	{
		return _apply_token.segment;
	}

	ScopeStatement::ScopeStatement(Token&& scope_token, Token&& symbol_token, BuiltinSymbol specifier, Expression* count)
		: _scope_token(std::move(scope_token)), _symbol_token(std::move(symbol_token)), _specifier(specifier), _count(count)
	{
	}

	void ScopeStatement::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			if (!is_scope_symbol(_specifier))
				ctx.add_semantic_error(_symbol_token.segment, "unrecognized scope symbol");

			if (_count)
			{
				_count->analyse(ctx, pass);

				if (!_count->evaltype(ctx).can_cast_explicit(DataType::Int()))
					ctx.add_semantic_error(_count->segment(), "cannot convert to " + DataType::Int().repr());
			}
		}
	}

	ExecutionFlow ScopeStatement::execute(Runtime& runtime) const
	{
		runtime.search_scope() = scope(runtime);
		return {};
	}

	ScriptSegment ScopeStatement::impl_segment() const
	{
		return _scope_token.segment;
	}

	Scope ScopeStatement::scope(Runtime& runtime) const
	{
		if (_specifier == BuiltinSymbol::Page)
		{
			if (_count)
				throw LxError::segment_error(_count->segment(), ErrorType::Runtime, "unexpected operand for $page scope");
			else
				return Scope(std::nullopt);
		}
		else if (_specifier == BuiltinSymbol::Line)
		{
			if (_count)
				throw LxError::segment_error(_count->segment(), ErrorType::Runtime, "unexpected operand for $line scope");
			else
				return Scope(1u);
		}
		else if (_specifier == BuiltinSymbol::Lines)
		{
			if (_count)
			{
				int c = _count->evaluate(runtime).consume_as<Int>(eval_context(runtime)).value();
				if (c > 0)
					return Scope(c);
				else
				{
					std::stringstream ss;
					ss << "'count' evaluated to " << c << ": but should be a positive " << DataType::Int();
					throw LxError::segment_error(_count->segment(), ErrorType::Runtime, ss.str());
				}
			}
			else
				throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "expected 'count' operand for $lines scope");
		}
		else
			throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "unrecognized scope specifier");
	}

	PagePush::PagePush(Token&& page_token, Expression& page)
		: _page_token(std::move(page_token)), _page(page)
	{
	}

	void PagePush::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
		if (pass == AnalysisPass::Validation)
		{
			_page.analyse(ctx, pass);

			if (!_page.evaltype(ctx).is_pageable())
			{
				std::stringstream ss;
				ss << "cannot push " << _page.evaltype(ctx) << " to page stack";
				ctx.add_semantic_error(_page.segment(), ss.str());
			}
		}
	}

	ExecutionFlow PagePush::execute(Runtime& runtime) const
	{
		runtime.push_page(_page.evaluate(runtime), segment());
		return {};
	}

	ScriptSegment PagePush::impl_segment() const
	{
		return _page_token.segment;
	}

	PagePop::PagePop(Token&& page_token)
		: _page_token(std::move(page_token))
	{
	}

	void PagePop::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
	}

	ExecutionFlow PagePop::execute(Runtime& runtime) const
	{
		runtime.pop_page(segment());
		return {};
	}

	ScriptSegment PagePop::impl_segment() const
	{
		return _page_token.segment;
	}

	PageClearStack::PageClearStack(Token&& page_token)
		: _page_token(std::move(page_token))
	{
	}

	void PageClearStack::impl_analyse(SemanticContext& ctx, AnalysisPass pass)
	{
	}

	ExecutionFlow PageClearStack::execute(Runtime& runtime) const
	{
		runtime.clear_page_stack();
		return {};
	}

	ScriptSegment PageClearStack::impl_segment() const
	{
		return _page_token.segment;
	}
}
