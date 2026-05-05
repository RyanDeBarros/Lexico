#include "ast.h"

#include "types/accessor.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	namespace errors
	{
		static const char* DOES_NOT_RESOLVE = "expression does not resolve to";
	}

	static DataType assert_implicitly_casts(const SemanticContext& ctx, const Expression& expr, DataType to)
	{
		if (can_cast_implicit(expr.evaltype(ctx), to))
			return to;

		std::stringstream ss;
		ss << errors::DOES_NOT_RESOLVE << to;
		throw LxError::segment_error(expr.segment(), ErrorType::Internal, ss.str());
	}

	static void validate_implicitly_casts(const SemanticContext& ctx, const Expression& expr, DataType to)
	{
		if (!can_cast_implicit(expr.evaltype(ctx), to))
		{
			std::stringstream ss;
			ss << errors::DOES_NOT_RESOLVE << to;
			ctx.add_semantic_error(expr.segment(), ss.str());
		}
	}

	void UpflowInfo::merge_loop_control(const UpflowInfo& other)
	{
		may_break |= other.may_break;
		may_continue |= other.may_continue;
		breaks.insert(breaks.end(), other.breaks.begin(), other.breaks.end());
		continues.insert(continues.end(), other.continues.begin(), other.continues.end());
	}

	bool ASTNode::validated() const
	{
		return _validated;
	}

	void ASTNode::accept(ASTVisitor& visitor)
	{
		visitor.pre_visit(*this);
		traverse(visitor);
		visitor.post_visit(*this);
	}

	UpflowInfo ASTNode::upflow(const SemanticContext& ctx)
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

	UpflowInfo ASTNode::impl_upflow(const SemanticContext& ctx)
	{
		return {};
	}

	ScriptSegment ASTNode::segment() const
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

	void Block::pre_analyse(SemanticContext& ctx)
	{
		ctx.push_local_scope(isolated());
	}

	void Block::post_analyse(SemanticContext& ctx)
	{
		ctx.pop_local_scope();
	}

	ExecutionFlow Block::execute(Runtime& env) const
	{
		ExecutionFlow flow{};
		env.push_local_scope(isolated());
		for (const ASTNode* node : _children)
		{
			auto result = node->execute(env);
			if (result.type != ExecutionFlow::Type::Normal)
			{
				flow = std::move(result);
				break;
			}
		}
		env.pop_local_scope();
		return flow;
	}

	void Block::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		for (ASTNode* node : _children)
			node->accept(visitor);
	}

	UpflowInfo Block::impl_upflow(const SemanticContext& ctx)
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

	void IsolationBlock::post_analyse(SemanticContext& ctx)
	{
		auto flow = block_upflow(ctx);

		for (BreakStatement* b : flow.breaks)
			ctx.add_semantic_error(b->segment(), "no outer loop to break out of");

		for (ContinueStatement* c : flow.continues)
			ctx.add_semantic_error(c->segment(), "no outer loop to continue to");

		Block::post_analyse(ctx);
	}

	bool IsolationBlock::isolated() const
	{
		return true;
	}

	UpflowInfo IsolationBlock::impl_upflow(const SemanticContext& ctx)
	{
		return {};
	}

	UpflowInfo IsolationBlock::block_upflow(const SemanticContext& ctx)
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

	void ASTRoot::pre_analyse(SemanticContext& ctx)
	{
		IsolationBlock::pre_analyse(ctx);

		_validated = true;
	}

	ScriptSegment ASTRoot::impl_segment() const
	{
		return _start_token.segment;
	}

	ExecutionFlow Expression::execute(Runtime& env) const
	{
		return {};
	}

	DataType Expression::evaltype(const SemanticContext& ctx) const
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

	void VariableDeclaration::pre_analyse(SemanticContext& ctx)
	{
		if (_global && ctx.scope_depth() > 0)
			ctx.add_semantic_error(_identifier, "cannot declare global variable inside a scope");
		else if (auto ln = ctx.identifier_first_decl_line_number(_identifier.lexeme, _global ? Namespace::Unknown : Namespace::Local))
			ctx.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(*ln));
		else
			_validated = true;
	}

	void VariableDeclaration::post_analyse(SemanticContext& ctx)
	{
		ctx.register_variable(_identifier.lexeme, _expression.evaltype(ctx), _identifier.segment.start_line, _global ? Namespace::Global : Namespace::Local);
	}

	ExecutionFlow VariableDeclaration::execute(Runtime& env) const
	{
		env.register_variable(_identifier.lexeme, _expression.evaluate(env).dp(), _global ? Namespace::Global : Namespace::Local);
		return {};
	}

	void VariableDeclaration::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment VariableDeclaration::impl_segment() const
	{
		return _identifier.segment.combined_right(_expression.segment());
	}

	bool VariableDeclaration::is_global() const
	{
		return _global;
	}

	// TODO allow assigning to a MemberAccessExpression/SubscriptExpression, not just identifier -> parser should parse expression up until first '='
	VariableAssignment::VariableAssignment(Token&& identifier, Expression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableAssignment::pre_analyse(SemanticContext& ctx)
	{
		if (ctx.registered_variable(_identifier.lexeme, Namespace::Unknown).has_value())
			_validated = true;
		else
			ctx.add_semantic_error(_identifier, "variable is not declared in scope");
	}

	void VariableAssignment::post_analyse(SemanticContext& ctx)
	{
		auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown);
		if (!can_cast_implicit(_expression.evaltype(ctx), var->type))
		{
			std::stringstream ss;
			ss << "cannot assign variable of type " << var->type << " to expression of type " << _expression.evaltype(ctx);
			ctx.add_semantic_error(segment(), ss.str());
		}
	}

	ExecutionFlow VariableAssignment::execute(Runtime& env) const
	{
		env.registered_variable(_identifier.lexeme, Namespace::Unknown).ref().set(_expression.evaluate(env).dp());
		return {};
	}

	void VariableAssignment::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment VariableAssignment::impl_segment() const
	{
		return _identifier.segment.combined_right(_expression.segment());
	}

	GlobalMatchesAssignment::GlobalMatchesAssignment(Token&& percent_token, Expression& expression)
		: _percent_token(std::move(percent_token)), _expression(expression)
	{
	}

	void GlobalMatchesAssignment::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void GlobalMatchesAssignment::post_analyse(SemanticContext& ctx)
	{
		if (!can_cast_implicit(_expression.evaltype(ctx), DataType::Matches))
		{
			std::stringstream ss;
			ss << "cannot assign `%` to expression of type " << _expression.evaltype(ctx);
			ctx.add_semantic_error(segment(), ss.str());
		}
	}

	ExecutionFlow GlobalMatchesAssignment::execute(Runtime& env) const
	{
		env.global_matches() = _expression.evaluate(env).dp().cast_move(DataType::Matches).get<Matches>();
		return {};
	}

	void GlobalMatchesAssignment::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment GlobalMatchesAssignment::impl_segment() const
	{
		return _percent_token.segment.combined_right(_expression.segment());
	}

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void LiteralExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void LiteralExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}
	
	Variable LiteralExpression::evaluate(Runtime& env) const
	{
		return env.temporary_variable(DataPoint::make_from_literal(data_type(_literal.type), _literal.resolved()));
	}

	DataType LiteralExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		return literal_type(_literal.type);
	}

	ScriptSegment LiteralExpression::impl_segment() const
	{
		return _literal.segment;
	}

	ListExpression::ListExpression(Token&& lbracket_token, Token&& rbracket_token, std::vector<Expression*>&& elements)
		: _lbracket_token(std::move(lbracket_token)), _rbracket_token(std::move(rbracket_token)), _elements(std::move(elements))
	{
	}
	
	void ListExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}
	
	void ListExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable ListExpression::evaluate(Runtime& env) const
	{
		std::vector<LxError> errors;
		Variable list_handle = env.temporary_variable(List());
		List& list = list_handle.ref().get<List>();

		for (const Expression* expr : _elements)
		{
			TypeVariant v = std::move(expr->evaluate(env).dp().variant());
			try
			{
				list.push(env.unnamed_variable(list_handle, std::move(v)));
			}
			catch (const LxError&)
			{
				errors.push_back(LxError::segment_error(expr->segment(), ErrorType::Runtime, "does not resolve to a public type"));
			}
		}

		if (errors.empty())
			return env.temporary_variable(std::move(list));
		else
			throw errors;
	}

	DataType ListExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		return DataType::List;
	}

	ScriptSegment ListExpression::impl_segment() const
	{
		return _lbracket_token.segment.combined_right(_rbracket_token.segment);
	}

	BinaryExpression::BinaryExpression(Token&& op, Expression& left, Expression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void BinaryExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void BinaryExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable BinaryExpression::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	void BinaryExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	DataType BinaryExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		if (auto type = lx::evaltype(op(), _left.evaltype(ctx), _right.evaltype(ctx)))
			return *type;
		else
		{
			std::stringstream ss;
			ss << ": operator not defined for types " << _left.evaltype(ctx) << " and " << _right.evaltype(ctx);
			ctx.add_semantic_error(_op, ss.str());
			return DataType::Void;
		}
	}

	ScriptSegment BinaryExpression::impl_segment() const
	{
		return _left.segment().combined_right(_right.segment());
	}

	BinaryOperator BinaryExpression::op() const
	{
		return binary_operator(_op.type);
	}

	MemberAccessExpression::MemberAccessExpression(Expression& object, Token&& member)
		: _object(object), _member_name(std::move(member))
	{
	}

	void MemberAccessExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void MemberAccessExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable MemberAccessExpression::evaluate(Runtime& env) const
	{
		const MemberSignature& m = member();
		if (m.is_data())
			return DataAccessor::invoke(_object.evaluate(env), env, m.identifier);
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member \"" << m.identifier << "\" is not data layout";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	void MemberAccessExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_object.accept(visitor);
	}

	DataType MemberAccessExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		return member(ctx).data_type();
	}

	ScriptSegment MemberAccessExpression::impl_segment() const
	{
		return _object.segment().combined_right(_member_name.segment);
	}

	const MemberSignature& MemberAccessExpression::member(const SemanticContext& ctx) const
	{
		if (_member)
			return *_member;

		if (const auto members = data_type_members(_object.evaltype(ctx)))
		{
			auto it = members->find(_member_name.lexeme);
			if (it != members->end())
			{
				const auto& member = it->second;
				if (_callable ? member.is_method() : member.is_data())
				{
					_member = member;
					return *_member;
				}
			}
		}

		std::stringstream ss;
		ss << "member " << _member_name.lexeme << " does not exist for type " << _object.evaltype(ctx);
		throw LxError::segment_error(_member_name.segment, ErrorType::Semantic, ss.str());
	}

	const MemberSignature& MemberAccessExpression::member() const
	{
		if (!_member)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member not set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			return *_member;
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

	void PrefixExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PrefixExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable PrefixExpression::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	void PrefixExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType PrefixExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		if (auto type = lx::evaltype(op(), _expr.evaltype(ctx)))
			return *type;
		else
		{
			std::stringstream ss;
			ss << ": operator not defined for type " << _expr.evaltype(ctx);
			ctx.add_semantic_error(_op, ss.str());
			return DataType::Void;
		}
	}

	ScriptSegment PrefixExpression::impl_segment() const
	{
		return _op.segment.combined_right(_expr.segment());
	}

	PrefixOperator PrefixExpression::op() const
	{
		return prefix_operator(_op.type);
	}

	AsExpression::AsExpression(Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void AsExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable AsExpression::evaluate(Runtime& env) const
	{
		return env.temporary_variable(_expr.evaluate(env).dp().cast_move(data_type(_type.type)));
	}

	void AsExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType AsExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		DataType return_type = data_type(_type.type);
		if (can_cast_explicit(_expr.evaltype(ctx), return_type))
			return return_type;
		else
		{
			std::stringstream ss;
			ss << "cannot convert from " << _expr.evaltype(ctx) << " to " << return_type;
			ctx.add_semantic_error(_type, ss.str());
			return DataType::Void;
		}
	}

	ScriptSegment AsExpression::impl_segment() const
	{
		return _expr.segment().combined_right(_type.segment);
	}

	SubscriptExpression::SubscriptExpression(Expression& container, Expression& subscript)
		: _container(container), _subscript(subscript)
	{
	}

	void SubscriptExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void SubscriptExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable SubscriptExpression::evaluate(Runtime& env) const
	{
		// TODO use constant const char* for "[]"
		return MethodAccessor::invoke(_container.evaluate(env), env, "[]", { _subscript.evaluate(env) });
	}

	void SubscriptExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_container.accept(visitor);
		_subscript.accept(visitor);
	}

	DataType SubscriptExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		return member(ctx).return_type({_subscript.evaltype(ctx)}).value();
	}

	ScriptSegment SubscriptExpression::impl_segment() const
	{
		return _container.segment().combined_right(_subscript.segment());
	}

	const MemberSignature& SubscriptExpression::member(const SemanticContext& ctx) const
	{
		if (_member)
			return *_member;

		if (const auto members = data_type_members(_container.evaltype(ctx)))
		{
			auto it = members->find("[]");
			if (it != members->end())
			{
				const auto& member = it->second;
				if (member.is_method() && member.return_type({ _subscript.evaltype(ctx) }).has_value())
				{
					_member = member;
					return *_member;
				}
			}
		}

		std::stringstream ss;
		ss << _container.evaltype(ctx) << " does not support [] with index type " << _subscript.evaltype(ctx);
		throw LxError::segment_error(_container.segment(), ErrorType::Semantic, ss.str());
	}

	const MemberSignature& SubscriptExpression::member() const
	{
		if (!_member)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member not set";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
			return *_member;
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void VariableExpression::pre_analyse(SemanticContext& ctx)
	{
		if (ctx.registered_variable(_identifier.lexeme, Namespace::Unknown).has_value())
			_validated = true;
		else
			ctx.add_semantic_error(_identifier, "variable is not declared in scope");
	}

	void VariableExpression::post_analyse(SemanticContext& ctx)
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

	Variable VariableExpression::evaluate(Runtime& env) const
	{
		// TODO should return a reference, not temporary
		return env.temporary_variable(Void());
	}

	DataType VariableExpression::impl_evaltype(const SemanticContext& ctx) const
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

	BuiltinSymbolExpression::BuiltinSymbolExpression(Token&& symbol_token, BuiltinSymbol builtin_symbol)
		: _symbol_token(std::move(symbol_token)), _builtin_symbol(builtin_symbol)
	{
	}

	void BuiltinSymbolExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void BuiltinSymbolExpression::post_analyse(SemanticContext& ctx)
	{
		evaltype(ctx);
	}

	Variable BuiltinSymbolExpression::evaluate(Runtime& env) const
	{
		switch (evaltype())
		{
		case DataType::Pattern:
			return env.temporary_variable(Pattern::make_from_symbol(_builtin_symbol));
		case DataType::Matches:
			return env.global_matches_handle();
		case DataType::_Marker:
			return env.temporary_variable(Marker(marker(_builtin_symbol)));
		case DataType::_Scope:
			throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "unexpected scope symbol");
		case DataType::_Color:
			throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "unexpected color symbol");
		default:
			throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "unrecognized symbol");
		}
	}

	DataType BuiltinSymbolExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		return data_type(_builtin_symbol);
	}

	ScriptSegment BuiltinSymbolExpression::impl_segment() const
	{
		return _symbol_token.segment;
	}

	FunctionCallExpression::FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args, Token&& closing_paren)
		: _identifier(std::move(identifier)), _args(std::move(args)), _closing_paren(std::move(closing_paren))
	{
	}

	void FunctionCallExpression::pre_analyse(SemanticContext& ctx)
	{
		if (ctx.registered_function_calls(_identifier.lexeme, Namespace::Unknown).empty())
			ctx.add_semantic_error(_identifier, "function not declared in scope");
		else
			_validated = true;
	}

	void FunctionCallExpression::post_analyse(SemanticContext& ctx)
	{
		auto argtypes = arg_types(ctx);
		if (!ctx.registered_function(_identifier.lexeme, argtypes, Namespace::Unknown))
		{
			std::stringstream ss;
			ss << "no declaration of '" << _identifier.lexeme << "' matches the arguemnt types (";
			for (size_t i = 0; i < argtypes.size(); ++i)
			{
				ss << argtypes[i];
				if (i + 1 < argtypes.size())
					ss << ", ";
			}
			ss << ")";
			ctx.add_semantic_error(segment(), ss.str());
		}
	}

	Variable FunctionCallExpression::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	void FunctionCallExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		for (Expression* arg : _args)
			arg->accept(visitor);
	}

	DataType FunctionCallExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		if (auto fn = ctx.registered_function(_identifier.lexeme, arg_types(ctx), Namespace::Unknown))
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

	std::vector<DataType> FunctionCallExpression::arg_types(const SemanticContext& ctx) const
	{
		std::vector<DataType> args(_args.size());
		for (size_t i = 0; i < args.size(); ++i)
			args[i] = _args[i]->evaltype(ctx);
		return args;
	}

	MethodCallExpression::MethodCallExpression(MemberAccessExpression& member, std::vector<Expression*>&& args, Token&& closing_paren)
		: _member(member), _args(std::move(args)), _closing_paren(std::move(closing_paren))
	{
		_member.set_callable(true);
	}

	void MethodCallExpression::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void MethodCallExpression::post_analyse(SemanticContext& ctx)
	{
		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(segment(), e.message());
		}
	}

	Variable MethodCallExpression::evaluate(Runtime& env) const
	{
		const MemberSignature& m = _member.member();
		if (m.is_method())
		{
			std::vector<Variable> args;
			for (const Expression* expr : _args)
				args.push_back(expr->evaluate(env));
			return MethodAccessor::invoke(_member.object().evaluate(env), env, m.identifier, args);
		}
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": member \"" << m.identifier << "\" is not method";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	void MethodCallExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_member.accept(visitor);
		for (Expression* arg : _args)
			arg->accept(visitor);
	}

	bool MethodCallExpression::imperative() const
	{
		return true;
	}

	DataType MethodCallExpression::impl_evaltype(const SemanticContext& ctx) const
	{
		const auto& m = _member.member(ctx);
		if (m.is_method())
		{
			std::vector<DataType> arg_types;
			for (Expression* arg : _args)
				arg_types.push_back(arg->evaltype(ctx));

			if (auto r = m.return_type(arg_types))
				return *r;

			std::stringstream ss;
			ss << "no overloads exist for '" << m.identifier << "' with arguments (";
			for (size_t i = 0; i < arg_types.size(); ++i)
			{
				ss << arg_types[i];
				if (i + 1 < arg_types.size())
					ss << ", ";
			}
			ss << ")";
			throw LxError(ErrorType::Internal, ss.str());
		}
		else
		{
			std::stringstream ss;
			ss << "'" << m.identifier << "' is not callable";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	ScriptSegment MethodCallExpression::impl_segment() const
	{
		return _member.segment().combined_right(_closing_paren.segment);
	}

	FunctionDefinition::FunctionDefinition(Token&& fn_token, Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, std::optional<Token>&& return_type)
		: _fn_token(std::move(fn_token)), _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	void FunctionDefinition::pre_analyse(SemanticContext& ctx)
	{
		if (auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown))
		{
			ctx.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(var->decl_line_number));
			return;
		}
		
		if (auto fn = ctx.registered_function(_identifier.lexeme, arg_types(), Namespace::Unknown))
		{
			ctx.add_semantic_error(_identifier, "function with matching argument types already declared on line " + std::to_string(fn->decl_line_number));
			return;
		}

		std::unordered_set<std::string_view> argnames;
		for (const auto& [_, identifier] : _arglist)
		{
			if (argnames.count(identifier.lexeme))
				ctx.add_semantic_error(identifier, "repeated argument identifier");
			else
				argnames.insert(identifier.lexeme);
		}

		if (argnames.size() != _arglist.size())
			return;

		ctx.register_function(_identifier.lexeme, return_type(), arg_types(), _identifier.segment.start_line, ctx.scope_depth() == 0 ? Namespace::Global : Namespace::Local);

		IsolationBlock::pre_analyse(ctx);

		for (size_t i = 0; i < _arglist.size(); ++i)
			ctx.register_variable(_arglist[i].second.lexeme, data_type(_arglist[i].first.type), _arglist[i].second.segment.start_line, Namespace::Local);

		_validated = true;
	}

	void FunctionDefinition::post_analyse(SemanticContext& ctx)
	{
		auto flow = block_upflow(ctx);

		if (return_type() != DataType::Void && !flow.always_returns)
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

		IsolationBlock::post_analyse(ctx);
	}

	ExecutionFlow FunctionDefinition::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	UpflowInfo FunctionDefinition::impl_upflow(const SemanticContext& ctx)
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
			types.push_back(data_type(arg.first.type));
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
		return _return_type ? data_type(_return_type->type) : DataType::Void;
	}

	ReturnStatement::ReturnStatement(Token&& return_token, Expression* expression)
		: _return_token(std::move(return_token)), _expression(expression)
	{
	}

	void ReturnStatement::pre_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	void ReturnStatement::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow ReturnStatement::execute(Runtime& env) const
	{
		if (_expression)
			return { .type = ExecutionFlow::Type::Return, .data = _expression->evaluate(env) };
		else
			return { .type = ExecutionFlow::Type::Return, .data = env.temporary_variable(Void()) };
	}

	void ReturnStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		if (_expression)
			_expression->accept(visitor);
	}

	UpflowInfo ReturnStatement::impl_upflow(const SemanticContext& ctx)
	{
		return { .always_returns = true, .live_returns = { this } };
	}

	ScriptSegment ReturnStatement::impl_segment() const
	{
		return _expression ? _expression->segment() : _return_token.segment;
	}

	DataType ReturnStatement::evaltype(const SemanticContext& ctx) const
	{
		return _expression ? _expression->evaltype(ctx) : DataType::Void;
	}

	IfConditional::IfConditional(Expression& condition)
		: _condition(condition)
	{
	}

	const Expression& IfConditional::condition() const
	{
		return _condition;
	}

	Expression& IfConditional::condition()
	{
		return _condition;
	}

	void IfConditional::set_fallback(IfFallbackBlock* fallback)
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

	ExecutionFlow IfConditional::execute(Runtime& env) const
	{
		if (_condition.evaluate(env).dp().move_as<Bool>().value())
			return Block::execute(env);
		else if (_fallback)
			return _fallback->execute(env);
		else
			return {};
	}

	UpflowInfo IfConditional::impl_upflow(const SemanticContext& ctx)
	{
		auto info = block_upflow(ctx);
		if (_fallback)
		{
			auto fallback_info = _fallback->upflow(ctx);
			info.live_returns.insert(info.live_returns.end(), fallback_info.live_returns.begin(), fallback_info.live_returns.end());
			info.dead_returns.insert(info.dead_returns.end(), fallback_info.dead_returns.begin(), fallback_info.dead_returns.end());
			info.always_returns &= fallback_info.always_returns;
			info.merge_loop_control(fallback_info);
		}
		else
			info.always_returns = false;
		return info;
	}

	UpflowInfo IfConditional::block_upflow(const SemanticContext& ctx)
	{
		if (!_block_upflow)
			_block_upflow = Block::impl_upflow(ctx);
		return *_block_upflow;
	}

	UpflowInfo IfConditional::block_upflow() const
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
		: IfConditional(condition), _if_token(std::move(if_token))
	{
	}

	void IfStatement::pre_analyse(SemanticContext& ctx)
	{
		Block::pre_analyse(ctx);
		_validated = true;
	}

	void IfStatement::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, condition(), DataType::Bool);
		Block::post_analyse(ctx);
	}

	void IfStatement::traverse(ASTVisitor& visitor)
	{
		condition().accept(visitor);
		Block::traverse(visitor);
		if (_fallback)
			_fallback->accept(visitor);
	}

	UpflowInfo IfStatement::impl_upflow(const SemanticContext& ctx)
	{
		return IfConditional::impl_upflow(ctx);
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
		: IfConditional(condition), _elif_token(std::move(elif_token))
	{
	}

	void ElifStatement::pre_analyse(SemanticContext& ctx)
	{
		Block::pre_analyse(ctx);
		_validated = true;
	}

	void ElifStatement::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, condition(), DataType::Bool);
		Block::post_analyse(ctx);
	}

	void ElifStatement::traverse(ASTVisitor& visitor)
	{
		condition().accept(visitor);
		Block::traverse(visitor);
		if (_fallback)
			_fallback->accept(visitor);
	}

	UpflowInfo ElifStatement::impl_upflow(const SemanticContext& ctx)
	{
		return IfConditional::impl_upflow(ctx);
	}

	ScriptSegment ElifStatement::impl_segment() const
	{
		return _elif_token.segment;
	}

	bool ElifStatement::isolated() const
	{
		return false;
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

	Loop::Loop(Token&& loop_token)
		: _loop_token(std::move(loop_token))
	{
	}

	void Loop::pre_analyse(SemanticContext& ctx)
	{
		Block::pre_analyse(ctx);

		_validated = true;
	}

	void Loop::post_analyse(SemanticContext& ctx)
	{
		auto subflow = block_upflow(ctx);

		for (BreakStatement* b : subflow.breaks)
			b->attach_loop(this);

		for (ContinueStatement* c : subflow.continues)
			c->attach_loop(this);

		Block::post_analyse(ctx);
	}

	UpflowInfo Loop::impl_upflow(const SemanticContext& ctx)
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

	UpflowInfo Loop::block_upflow(const SemanticContext& ctx)
	{
		if (!_block_upflow)
			_block_upflow = Block::impl_upflow(ctx);
		return *_block_upflow;
	}

	WhileLoop::WhileLoop(Token&& loop_token, Expression& condition)
		: Loop(std::move(loop_token)), _condition(condition)
	{
	}

	void WhileLoop::pre_analyse(SemanticContext& ctx)
	{
		Loop::pre_analyse(ctx);
	}

	void WhileLoop::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, _condition, DataType::Bool);
		Loop::post_analyse(ctx);
	}

	ExecutionFlow WhileLoop::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	void WhileLoop::traverse(ASTVisitor& visitor)
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
	}

	bool WhileLoop::isolated() const
	{
		return false;
	}

	ForLoop::ForLoop(Token&& loop_token, Token&& iterator, Expression& iterable)
		: Loop(std::move(loop_token)), _iterator(std::move(iterator)), _iterable(iterable)
	{
	}

	void ForLoop::pre_analyse(SemanticContext& ctx)
	{
		Loop::pre_analyse(ctx);
		ctx.registered_variable(_iterator.lexeme, Namespace::Local);
	}

	void ForLoop::post_analyse(SemanticContext& ctx)
	{
		if (!is_iterable(_iterable.evaltype(ctx)))
		{
			std::stringstream ss;
			ss << _iterable.evaltype(ctx) << " is not iterable";
			ctx.add_semantic_error(_iterable.segment(), ss.str());
		}

		Loop::post_analyse(ctx);
	}

	ExecutionFlow ForLoop::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	void ForLoop::traverse(ASTVisitor& visitor)
	{
		_iterable.accept(visitor);
		Block::traverse(visitor);
	}

	bool ForLoop::isolated() const
	{
		return false;
	}

	BreakStatement::BreakStatement(Token&& break_token)
		: _break_token(std::move(break_token))
	{
	}

	void BreakStatement::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void BreakStatement::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow BreakStatement::execute(Runtime& env) const
	{
		return { .type = ExecutionFlow::Type::Break };
	}

	UpflowInfo BreakStatement::impl_upflow(const SemanticContext& ctx)
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

	void ContinueStatement::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void ContinueStatement::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow ContinueStatement::execute(Runtime& env) const
	{
		return { .type = ExecutionFlow::Type::Continue };
	}

	UpflowInfo ContinueStatement::impl_upflow(const SemanticContext& ctx)
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

	void LogStatement::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void LogStatement::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow LogStatement::execute(Runtime& env) const
	{
		// TODO
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

	void HighlightStatement::pre_analyse(SemanticContext& ctx)
	{
		if (!_color_token || data_type(_color) == DataType::_Color)
			_validated = true;
		else
			ctx.add_semantic_error(*_color_token, "symbol is not a color");
	}

	void HighlightStatement::post_analyse(SemanticContext& ctx)
	{
		if (_highlightable && !is_highlightable(_highlightable->evaltype(ctx)))
		{
			std::stringstream ss;
			ss << _highlightable->evaltype(ctx) << " is not highlightable";
			ctx.add_semantic_error(_highlightable->segment(), ss.str());
		}
	}

	ExecutionFlow HighlightStatement::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	void HighlightStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		if (_highlightable)
			_highlightable->accept(visitor);
	}

	ScriptSegment HighlightStatement::impl_segment() const
	{
		return _highlight_token.segment;
	}

	DeletePattern::DeletePattern(Token&& delete_token, Token&& identifier)
		: _delete_token(std::move(delete_token)), _identifier(std::move(identifier))
	{
	}

	void DeletePattern::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void DeletePattern::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow DeletePattern::execute(Runtime& env) const
	{
		// TODO
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

	void PatternDeclaration::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PatternDeclaration::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow PatternDeclaration::execute(Runtime& env) const
	{
		// TODO
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

	void RepeatOperation::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void RepeatOperation::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, _range, DataType::IRange);

		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(_expression.segment(), e.message());
		}
	}

	Variable RepeatOperation::evaluate(Runtime& env) const
	{
		// TODO should return a reference, not temporary
		return env.temporary_variable(Void());
	}

	void RepeatOperation::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
		_range.accept(visitor);
	}

	DataType RepeatOperation::impl_evaltype(const SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern);
	}
	
	ScriptSegment RepeatOperation::impl_segment() const
	{
		return _expression.segment().combined_right(_range.segment());
	}

	SimpleRepeatOperation::SimpleRepeatOperation(Expression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}

	void SimpleRepeatOperation::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void SimpleRepeatOperation::post_analyse(SemanticContext& ctx)
	{
		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(_expression.segment(), e.message());
		}
	}

	Variable SimpleRepeatOperation::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	void SimpleRepeatOperation::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	DataType SimpleRepeatOperation::impl_evaltype(const SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern);
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

	void PatternBackRef::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PatternBackRef::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	Variable PatternBackRef::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	DataType PatternBackRef::impl_evaltype(const SemanticContext& ctx) const
	{
		return DataType::Pattern;
	}

	ScriptSegment PatternBackRef::impl_segment() const
	{
		return _ref_token.segment.combined_right(_identifier.segment);
	}

	PatternLazy::PatternLazy(Token&& lazy_token, Expression& expression)
		: _lazy_token(std::move(lazy_token)), _expression(expression)
	{
	}

	void PatternLazy::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PatternLazy::post_analyse(SemanticContext& ctx)
	{
		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(_expression.segment(), e.message());
		}
	}

	Variable PatternLazy::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	void PatternLazy::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	DataType PatternLazy::impl_evaltype(const SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern);
	}

	ScriptSegment PatternLazy::impl_segment() const
	{
		return _lazy_token.segment.combined_right(_expression.segment());
	}

	PatternCapture::PatternCapture(Token&& capture_token, Token&& identifier, Expression& expression)
		: _capture_token(std::move(capture_token)), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void PatternCapture::pre_analyse(SemanticContext& ctx)
	{
		if (_identifier.lexeme != UNNAMED_CAP_ID)
		{
			if (auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown))
			{
				std::stringstream ss;
				ss << "variable already declared on line " << var->decl_line_number;
				ctx.add_semantic_error(_identifier.segment, ss.str());
			}
			else
				_validated = true;
		}
		else
			_validated = true;
	}

	void PatternCapture::post_analyse(SemanticContext& ctx)
	{
		ctx.register_variable(_identifier.lexeme, DataType::CapId, _identifier.segment.start_line, Namespace::Global);

		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(_expression.segment(), e.message());
		}
	}

	Variable PatternCapture::evaluate(Runtime& env) const
	{
		// TODO
		return env.temporary_variable(Void());
	}

	void PatternCapture::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	DataType PatternCapture::impl_evaltype(const SemanticContext& ctx) const
	{
		return assert_implicitly_casts(ctx, _expression, DataType::Pattern);
	}

	ScriptSegment PatternCapture::impl_segment() const
	{
		return _capture_token.segment.combined_right(_expression.segment());
	}

	AppendStatement::AppendStatement(Token&& append_token, Expression& expression)
		: _append_token(std::move(append_token)), _expression(expression)
	{
	}

	void AppendStatement::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void AppendStatement::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, _expression, DataType::Pattern);
	}

	ExecutionFlow AppendStatement::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	void AppendStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment AppendStatement::impl_segment() const
	{
		return _append_token.segment;
	}

	FindStatement::FindStatement(Token&& find_token, Expression& pattern)
		: _find_token(std::move(find_token)), _pattern(pattern)
	{
	}

	void FindStatement::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void FindStatement::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, _pattern, DataType::Pattern);
	}

	ExecutionFlow FindStatement::execute(Runtime& env) const
	{
		// TODO
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

	void FilterStatement::pre_analyse(SemanticContext& ctx)
	{
		if (auto fn = ctx.registered_function(_identifier.lexeme, { DataType::Match }, Namespace::Unknown))
		{
			if (fn->return_type == DataType::Bool)
				_validated = true;
			else
			{
				std::stringstream ss;
				ss << "function on line " << fn->decl_line_number << " is not a match predicate: expected signature (" << DataType::Match << ") -> " << DataType::Bool;
				ctx.add_semantic_error(_identifier.segment, ss.str());
			}
		}
		else
		{
			std::stringstream ss;
			ss << "no matching predicate function: expected signature (" << DataType::Match << ") -> " << DataType::Bool;
			ctx.add_semantic_error(_identifier.segment, ss.str());
		}
	}

	void FilterStatement::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow FilterStatement::execute(Runtime& env) const
	{
		// TODO
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

	void ReplaceStatement::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void ReplaceStatement::post_analyse(SemanticContext& ctx)
	{
		validate_implicitly_casts(ctx, _match, DataType::Match);
		validate_implicitly_casts(ctx, _string, DataType::String);
	}

	ExecutionFlow ReplaceStatement::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	void ReplaceStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_match.accept(visitor);
		_string.accept(visitor);
	}

	ScriptSegment ReplaceStatement::impl_segment() const
	{
		return _replace_token.segment;
	}
	
	ApplyStatement::ApplyStatement(Token&& apply_token, Token&& identifier)
		: _apply_token(std::move(apply_token)), _identifier(std::move(identifier))
	{
	}

	void ApplyStatement::pre_analyse(SemanticContext& ctx)
	{
		if (auto fn = ctx.registered_function(_identifier.lexeme, { DataType::Match }, Namespace::Unknown))
		{
			if (fn->return_type == DataType::String)
				_validated = true;
			else
			{
				std::stringstream ss;
				ss << "found function on line " << fn->decl_line_number << ": expected signature (" << DataType::Match << ") -> " << DataType::String;
				ctx.add_semantic_error(_identifier.segment, ss.str());
			}
		}
		else
		{
			std::stringstream ss;
			ss << "no matching function: expected signature (" << DataType::Match << ") -> " << DataType::String;
			ctx.add_semantic_error(_identifier.segment, ss.str());
		}
	}

	void ApplyStatement::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow ApplyStatement::execute(Runtime& env) const
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

	void ScopeStatement::pre_analyse(SemanticContext& ctx)
	{
		if (data_type(_specifier) == DataType::_Scope)
			_validated = true;
		else
			ctx.add_semantic_error(_symbol_token.segment, "unrecognized scope symbol");
	}

	void ScopeStatement::post_analyse(SemanticContext& ctx)
	{
		if (_count)
		{
			const auto range_type = _count->evaltype(ctx);
			if (!can_cast_explicit(range_type, DataType::Int))
				ctx.add_semantic_error(_count->segment(), "cannot convert to " + friendly_name(DataType::Int));
		}
	}

	ExecutionFlow ScopeStatement::execute(Runtime& env) const
	{
		env.search_scope() = scope(env);
		return {};
	}

	void ScopeStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		if (_count)
			_count->accept(visitor);
	}
	
	Scope ScopeStatement::scope(Runtime& env) const
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
				int c = _count->evaluate(env).dp().move_as<Int>().value();
				if (c > 0)
					return Scope(c);
				else
				{
					std::stringstream ss;
					ss << "'count' evaluated to " << c << ": but should be a positive " << DataType::Int;
					throw LxError::segment_error(_count->segment(), ErrorType::Runtime, ss.str());
				}
			}
			else
				throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "expected 'count' operand for $lines scope");
		}
		else
			throw LxError::segment_error(_symbol_token.segment, ErrorType::Runtime, "unrecognized scope specifier");
	}

	ScriptSegment ScopeStatement::impl_segment() const
	{
		return _scope_token.segment;
	}

	PagePush::PagePush(Token&& page_token, Expression& page)
		: _page_token(std::move(page_token)), _page(page)
	{
	}

	void PagePush::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PagePush::post_analyse(SemanticContext& ctx)
	{
		if (!is_pageable(_page.evaltype(ctx)))
		{
			std::stringstream ss;
			ss << "cannot push " << _page.evaltype(ctx) << " to page stack";
			ctx.add_semantic_error(_page.segment(), ss.str());
		}
	}

	ExecutionFlow PagePush::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	ScriptSegment PagePush::impl_segment() const
	{
		return _page_token.segment;
	}

	void PagePush::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_page.accept(visitor);
	}

	PagePop::PagePop(Token&& page_token)
		: _page_token(std::move(page_token))
	{
	}

	void PagePop::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PagePop::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow PagePop::execute(Runtime& env) const
	{
		// TODO
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

	void PageClearStack::pre_analyse(SemanticContext& ctx)
	{
		_validated = true;
	}

	void PageClearStack::post_analyse(SemanticContext& ctx)
	{
		// NOP
	}

	ExecutionFlow PageClearStack::execute(Runtime& env) const
	{
		// TODO
		return {};
	}

	ScriptSegment PageClearStack::impl_segment() const
	{
		return _page_token.segment;
	}
}
