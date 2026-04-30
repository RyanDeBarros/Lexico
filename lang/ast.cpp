#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
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

	UpflowInfo ASTNode::upflow(const ResolutionContext& ctx)
	{
		if (!_upflow)
			_upflow = impl_upflow(ctx);
		return *_upflow;
	}

	UpflowInfo ASTNode::impl_upflow(const ResolutionContext& ctx)
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

	void Block::pre_analyse(ResolutionContext& ctx)
	{
		ctx.push_local_scope(isolated());
	}

	void Block::post_analyse(ResolutionContext& ctx)
	{
		ctx.pop_local_scope();
	}

	void Block::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		for (ASTNode* node : _children)
			node->accept(visitor);
	}

	UpflowInfo Block::impl_upflow(const ResolutionContext& ctx)
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

				livecode = !info.always_returns; // TODO not the best indicator
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

	ASTRoot::ASTRoot(Token&& start_token)
		: _start_token(std::move(start_token))
	{
	}

	void ASTRoot::pre_analyse(ResolutionContext& ctx)
	{
		Block::pre_analyse(ctx);

		_validated = true;
	}

	void ASTRoot::post_analyse(ResolutionContext& ctx)
	{
		auto flow = upflow(ctx);
		for (BreakStatement* b : flow.breaks)
			ctx.add_semantic_error(b->segment(), "no outer loop to break out of");
		for (ContinueStatement* c : flow.continues)
			ctx.add_semantic_error(c->segment(), "no outer loop to continue to");

		Block::post_analyse(ctx);
	}

	bool ASTRoot::isolated() const
	{
		return true;
	}

	ScriptSegment ASTRoot::impl_segment() const
	{
		return _start_token.segment;
	}

	DataType Expression::evaltype(const ResolutionContext& ctx) const
	{
		if (!_validated)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": node is not _validated";
			throw LxError(ErrorType::Internal, ss.str());
		}
		if (!_evaltype)
			_evaltype = impl_evaltype(ctx);
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

	void VariableDeclaration::pre_analyse(ResolutionContext& ctx)
	{
		if (_global && ctx.scope_depth() > 0)
			ctx.add_semantic_error(_identifier, "cannot declare global variable inside a scope");
		else if (auto ln = ctx.identifier_first_decl_line_number(_identifier.lexeme, _global ? Namespace::Unknown : Namespace::Local))
			ctx.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(*ln));
		else
			_validated = true;
	}

	void VariableDeclaration::post_analyse(ResolutionContext& ctx)
	{
		ctx.register_variable(_identifier.lexeme, _expression.evaltype(ctx), _identifier.segment.start_line, _global ? Namespace::Global : Namespace::Local);
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

	VariableAssignment::VariableAssignment(Token&& identifier, Expression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableAssignment::pre_analyse(ResolutionContext& ctx)
	{
		if (auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown))
			_validated = true;
		else
			ctx.add_semantic_error(_identifier, "variable is not declared in scope");
	}

	void VariableAssignment::post_analyse(ResolutionContext& ctx)
	{
		auto var = ctx.registered_variable(_identifier.lexeme, Namespace::Unknown);
		if (var->type != _expression.evaltype(ctx))
		{
			std::stringstream ss;
			ss << "cannot assign variable of type " << var->type << " to expression of type " << _expression.evaltype(ctx);
			ctx.add_semantic_error(_identifier.segment.combined_right(_expression.segment()), ss.str());
		}
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

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void LiteralExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void LiteralExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	DataType LiteralExpression::impl_evaltype(const ResolutionContext& ctx) const
	{
		switch (_literal.type)
		{
		case TokenType::Integer:
			return DataType::Int;
		case TokenType::Float:
			return DataType::Float;
		case TokenType::String:
			return DataType::String;
		case TokenType::Bool:
			return DataType::Bool;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": literal type " << static_cast<int>(_literal.type) << " is not convertible to data type";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	ScriptSegment LiteralExpression::impl_segment() const
	{
		return _literal.segment;
	}

	ListExpression::ListExpression(Token&& lbracket_token, Token&& rbracket_token, std::vector<Expression*>&& elements)
		: _lbracket_token(std::move(lbracket_token)), _rbracket_token(std::move(rbracket_token)), _elements(std::move(elements))
	{
	}
	
	void ListExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}
	
	void ListExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	DataType ListExpression::impl_evaltype(const ResolutionContext& ctx) const
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

	void BinaryExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void BinaryExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	void BinaryExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	DataType BinaryExpression::impl_evaltype(const ResolutionContext& ctx) const
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

	StandardBinaryOperator BinaryExpression::op() const
	{
		return standard_binary_operator(_op.type);
	}

	MemberAccessExpression::MemberAccessExpression(Expression& object, Token&& member)
		: _object(object), _member(std::move(member))
	{
	}

	void MemberAccessExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void MemberAccessExpression::post_analyse(ResolutionContext& ctx)
	{
		try
		{
			evaltype(ctx);
		}
		catch (const LxError& e)
		{
			ctx.add_semantic_error(_member.segment, e.message());
		}
	}

	void MemberAccessExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_object.accept(visitor);
	}

	DataType MemberAccessExpression::impl_evaltype(const ResolutionContext& ctx) const
	{
		return member(ctx).data_type();
	}

	ScriptSegment MemberAccessExpression::impl_segment() const
	{
		return _object.segment().combined_right(_member.segment);
	}

	const MemberSignature& MemberAccessExpression::member(const ResolutionContext& ctx) const
	{
		if (const auto members = data_type_members(_object.evaltype(ctx)))
			for (const auto& member : *members)
				if (member.identifier == _member.lexeme && member.is_data())
					return member;

		std::stringstream ss;
		ss << "data member " << _member.lexeme << " does not exist for type " << _object.evaltype(ctx);
		throw LxError(ErrorType::Internal, ss.str());
	}

	PrefixExpression::PrefixExpression(Token&& op, Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	void PrefixExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void PrefixExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	void PrefixExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType PrefixExpression::impl_evaltype(const ResolutionContext& ctx) const
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

	StandardPrefixOperator PrefixExpression::op() const
	{
		return standard_prefix_operator(_op.type);
	}

	AsExpression::AsExpression(Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void AsExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	void AsExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType AsExpression::impl_evaltype(const ResolutionContext& ctx) const
	{
		DataType return_type = data_type(_type.type);
		if (can_cast(_expr.evaltype(ctx), return_type))
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

	void SubscriptExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void SubscriptExpression::post_analyse(ResolutionContext& ctx)
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

	void SubscriptExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_container.accept(visitor);
		_subscript.accept(visitor);
	}

	DataType SubscriptExpression::impl_evaltype(const ResolutionContext& ctx) const
	{
		return member(ctx).return_type({_subscript.evaltype(ctx)}).value();
	}

	ScriptSegment SubscriptExpression::impl_segment() const
	{
		return _container.segment().combined_right(_subscript.segment());
	}

	const MemberSignature& SubscriptExpression::member(const ResolutionContext& ctx) const
	{
		if (const auto members = data_type_members(_container.evaltype(ctx)))
			for (const auto& member : *members)
				if (member.identifier == "[]" && member.is_method() && member.return_type({ _subscript.evaltype(ctx) }).has_value())
					return member;

		std::stringstream ss;
		ss << _container.evaltype(ctx) << " does not support [] with index type " << _subscript.evaltype(ctx);
		throw LxError(ErrorType::Internal, ss.str());
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void VariableExpression::pre_analyse(ResolutionContext& ctx)
	{
		if (ctx.registered_variable(_identifier.lexeme, Namespace::Unknown))
			_validated = true;
		else
			ctx.add_semantic_error(_identifier, "variable is not declared in scope");
	}

	void VariableExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	DataType VariableExpression::impl_evaltype(const ResolutionContext& ctx) const
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

	void BuiltinSymbolExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void BuiltinSymbolExpression::post_analyse(ResolutionContext& ctx)
	{
		evaltype(ctx);
	}

	DataType BuiltinSymbolExpression::impl_evaltype(const ResolutionContext& ctx) const
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

	void FunctionCallExpression::pre_analyse(ResolutionContext& ctx)
	{
		if (ctx.registered_function_calls(_identifier.lexeme, Namespace::Unknown).empty())
			ctx.add_semantic_error(_identifier, "function not declared in scope");
		else
			_validated = true;
	}

	void FunctionCallExpression::post_analyse(ResolutionContext& ctx)
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

	void FunctionCallExpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		for (Expression* arg : _args)
			arg->accept(visitor);
	}

	DataType FunctionCallExpression::impl_evaltype(const ResolutionContext& ctx) const
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

	std::vector<DataType> FunctionCallExpression::arg_types(const ResolutionContext& ctx) const
	{
		std::vector<DataType> args(_args.size());
		for (size_t i = 0; i < args.size(); ++i)
			args[i] = _args[i]->evaltype(ctx);
		return args;
	}

	MethodCallExpression::MethodCallExpression(MemberAccessExpression& member, std::vector<Expression*>&& args, Token&& closing_paren)
		: _member(member), _args(std::move(args)), _closing_paren(std::move(closing_paren))
	{
	}

	void MethodCallExpression::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void MethodCallExpression::post_analyse(ResolutionContext& ctx)
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

	DataType MethodCallExpression::impl_evaltype(const ResolutionContext& ctx) const
	{
		const auto m = _member.member(ctx);
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

	void FunctionDefinition::pre_analyse(ResolutionContext& ctx)
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

		Block::pre_analyse(ctx);

		for (size_t i = 0; i < _arglist.size(); ++i)
			ctx.register_variable(_arglist[i].second.lexeme, data_type(_arglist[i].first.type), _arglist[i].second.segment.start_line, Namespace::Local);

		_validated = true;
	}

	void FunctionDefinition::post_analyse(ResolutionContext& ctx)
	{
		auto flow = Block::impl_upflow(ctx);

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

		for (BreakStatement* b : flow.breaks)
			ctx.add_semantic_error(b->segment(), "no outer loop to break out of");

		for (ContinueStatement* c : flow.continues)
			ctx.add_semantic_error(c->segment(), "no outer loop to continue to");

		Block::post_analyse(ctx);
	}

	UpflowInfo FunctionDefinition::impl_upflow(const ResolutionContext& ctx)
	{
		return {};
	}

	ScriptSegment FunctionDefinition::impl_segment() const
	{
		return _fn_token.segment.combined_right(_identifier.segment);
	}

	bool FunctionDefinition::isolated() const
	{
		return true;
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

	void ReturnStatement::pre_analyse(ResolutionContext& ctx)
	{
	}

	void ReturnStatement::post_analyse(ResolutionContext& ctx)
	{
	}

	void ReturnStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		if (_expression)
			_expression->accept(visitor);
	}

	UpflowInfo ReturnStatement::impl_upflow(const ResolutionContext& ctx)
	{
		return { .always_returns = true, .live_returns = { this } };
	}

	ScriptSegment ReturnStatement::impl_segment() const
	{
		return _expression ? _expression->segment() : _return_token.segment;
	}

	DataType ReturnStatement::evaltype(const ResolutionContext& ctx) const
	{
		return _expression ? _expression->evaltype(ctx) : DataType::Void;
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

	UpflowInfo IfConditional::impl_upflow(const ResolutionContext& ctx)
	{
		auto info = Block::impl_upflow(ctx);
		if (_fallback)
		{
			auto fallback_info = _fallback->upflow(ctx);
			info.live_returns.insert(info.live_returns.end(), fallback_info.live_returns.begin(), fallback_info.live_returns.end());
			info.dead_returns.insert(info.dead_returns.end(), fallback_info.dead_returns.begin(), fallback_info.dead_returns.end());
			info.always_returns &= fallback_info.always_returns;
			info.merge_loop_control(fallback_info);
		}
		return info;
	}

	IfStatement::IfStatement(Token&& if_token, Expression& condition)
		: _if_token(std::move(if_token)), _condition(condition)
	{
	}

	void IfStatement::pre_analyse(ResolutionContext& ctx)
	{
		Block::pre_analyse(ctx);
		_validated = true;
	}

	void IfStatement::post_analyse(ResolutionContext& ctx)
	{
		if (_condition.evaltype(ctx) != DataType::Bool)
			ctx.add_semantic_error(_condition.segment(), "condition expression does not resolve to a 'bool'");
		Block::post_analyse(ctx);
	}

	void IfStatement::traverse(ASTVisitor& visitor)
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
		if (_fallback)
			_fallback->accept(visitor);
	}

	UpflowInfo IfStatement::impl_upflow(const ResolutionContext& ctx)
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
		: _elif_token(std::move(elif_token)), _condition(condition)
	{
	}

	void ElifStatement::pre_analyse(ResolutionContext& ctx)
	{
		Block::pre_analyse(ctx);
		_validated = true;
	}

	void ElifStatement::post_analyse(ResolutionContext& ctx)
	{
		if (_condition.evaltype(ctx) != DataType::Bool)
			ctx.add_semantic_error(_condition.segment(), "condition expression does not resolve to a 'bool'");
		Block::post_analyse(ctx);
	}

	void ElifStatement::traverse(ASTVisitor& visitor)
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
		if (_fallback)
			_fallback->accept(visitor);
	}

	UpflowInfo ElifStatement::impl_upflow(const ResolutionContext& ctx)
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

	void Loop::pre_analyse(ResolutionContext& ctx)
	{
		Block::pre_analyse(ctx);

		_validated = true;
	}

	void Loop::post_analyse(ResolutionContext& ctx)
	{
		auto subflow = Block::impl_upflow(ctx);
		for (BreakStatement* b : subflow.breaks)
			b->attach_loop(this);
		for (ContinueStatement* c : subflow.continues)
			c->attach_loop(this);

		Block::post_analyse(ctx);
	}

	UpflowInfo Loop::impl_upflow(const ResolutionContext& ctx)
	{
		auto info = Block::impl_upflow(ctx);
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
	
	WhileLoop::WhileLoop(Token&& loop_token, Expression& condition)
		: Loop(std::move(loop_token)), _condition(condition)
	{
	}

	void WhileLoop::pre_analyse(ResolutionContext& ctx)
	{
		Loop::pre_analyse(ctx);
	}

	void WhileLoop::post_analyse(ResolutionContext& ctx)
	{
		if (_condition.evaltype(ctx) != DataType::Bool)
			ctx.add_semantic_error(_condition.segment(), "condition expression does not resolve to a 'bool'");
		Loop::post_analyse(ctx);
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

	void ForLoop::pre_analyse(ResolutionContext& ctx)
	{
		Loop::pre_analyse(ctx);
		ctx.registered_variable(_iterator.lexeme, Namespace::Local);
	}

	void ForLoop::post_analyse(ResolutionContext& ctx)
	{
		if (!is_iterable(_iterable.evaltype(ctx)))
		{
			std::stringstream ss;
			ss << _iterable.evaltype(ctx) << " is not iterable";
			ctx.add_semantic_error(_iterable.segment(), ss.str());
		}

		Loop::post_analyse(ctx);
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

	void BreakStatement::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void BreakStatement::post_analyse(ResolutionContext& ctx)
	{
		// NOP
	}

	UpflowInfo BreakStatement::impl_upflow(const ResolutionContext& ctx)
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

	void ContinueStatement::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void ContinueStatement::post_analyse(ResolutionContext& ctx)
	{
		// NOP
	}

	UpflowInfo ContinueStatement::impl_upflow(const ResolutionContext& ctx)
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

	void LogStatement::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void LogStatement::post_analyse(ResolutionContext& ctx)
	{
		// NOP
	}

	ScriptSegment LogStatement::impl_segment() const
	{
		return _log_token.segment;
	}

	HighlightStatement::HighlightStatement(Token&& highlight_token, bool clear, Expression* highlightable, std::optional<Token>&& color_token, BuiltinSymbol color)
		: _highlight_token(std::move(highlight_token)), _clear(clear), _highlightable(highlightable), _color_token(std::move(color_token)), _color(color)
	{
	}

	void HighlightStatement::pre_analyse(ResolutionContext& ctx)
	{
		if (!_color_token || data_type(_color) == DataType::_Color)
			_validated = true;
		else
			ctx.add_semantic_error(*_color_token, "symbol is not a color");
	}

	void HighlightStatement::post_analyse(ResolutionContext& ctx)
	{
		if (_highlightable && !is_highlightable(_highlightable->evaltype(ctx)))
		{
			std::stringstream ss;
			ss << _highlightable->evaltype(ctx) << " is not highlightable";
			ctx.add_semantic_error(_highlightable->segment(), ss.str());
		}
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

	void DeletePattern::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void DeletePattern::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment DeletePattern::impl_segment() const
	{
		return _delete_token.segment.combined_right(_identifier.segment);
	}

	PatternDeclaration::PatternDeclaration(Token&& pattern_token, Token&& identifier)
		: _pattern_token(std::move(pattern_token)), _identifier(std::move(identifier))
	{
	}

	void PatternDeclaration::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternDeclaration::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment PatternDeclaration::impl_segment() const
	{
		return _pattern_token.segment.combined_right(_identifier.segment);
	}

	PatternSubexpression::PatternSubexpression(Expression& expr)
		: _expr(expr)
	{
	}

	void PatternSubexpression::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternSubexpression::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternSubexpression::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	ScriptSegment PatternSubexpression::impl_segment() const
	{
		return _expr.segment();
	}

	PatternLiteral::PatternLiteral(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void PatternLiteral::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternLiteral::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment PatternLiteral::impl_segment() const
	{
		return _literal.segment;
	}

	PatternIdentifier::PatternIdentifier(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternIdentifier::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternIdentifier::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment PatternIdentifier::impl_segment() const
	{
		return _identifier.segment;
	}

	PatternBuiltin::PatternBuiltin(Token&& symbol_token, BuiltinSymbol builtin_symbol)
		: _symbol_token(std::move(symbol_token)), _builtin_symbol(builtin_symbol)
	{
	}

	void PatternBuiltin::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternBuiltin::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment PatternBuiltin::impl_segment() const
	{
		return _symbol_token.segment;
	}

	PatternAs::PatternAs(PatternExpression& expression, Token&& type)
		: _expression(expression), _type(std::move(type))
	{
	}

	void PatternAs::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternAs::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternAs::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment PatternAs::impl_segment() const
	{
		return _expression.segment().combined_right(_type.segment);
	}

	PatternRepeat::PatternRepeat(PatternExpression& expression, Expression& range)
		: _expression(expression), _range(range)
	{
	}

	void PatternRepeat::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternRepeat::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternRepeat::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
		_range.accept(visitor);
	}

	ScriptSegment PatternRepeat::impl_segment() const
	{
		return _expression.segment().combined_right(_range.segment());
	}

	PatternSimpleRepeat::PatternSimpleRepeat(PatternExpression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}

	void PatternSimpleRepeat::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternSimpleRepeat::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternSimpleRepeat::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment PatternSimpleRepeat::impl_segment() const
	{
		return _expression.segment().combined_right(_op.segment);
	}

	PatternSimpleRepeatOperator PatternSimpleRepeat::op() const
	{
		return pattern_simple_repeat_operator(_op.type);
	}

	PatternPrefixOperation::PatternPrefixOperation(Token&& op, PatternExpression& expression)
		: _op(std::move(op)), _expression(expression)
	{
	}

	void PatternPrefixOperation::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternPrefixOperation::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternPrefixOperation::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment PatternPrefixOperation::impl_segment() const
	{
		return _op.segment.combined_right(_expression.segment());
	}

	PatternPrefixOperator PatternPrefixOperation::op() const
	{
		return pattern_prefix_operator(_op.type);
	}
	
	PatternBackRef::PatternBackRef(Token&& ref_token, Token&& identifier)
		: _ref_token(std::move(ref_token)), _identifier(std::move(identifier))
	{
	}

	void PatternBackRef::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternBackRef::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment PatternBackRef::impl_segment() const
	{
		return _ref_token.segment.combined_right(_identifier.segment);
	}

	PatternBinaryOperation::PatternBinaryOperation(Token&& op, PatternExpression& left, PatternExpression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void PatternBinaryOperation::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternBinaryOperation::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternBinaryOperation::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	ScriptSegment PatternBinaryOperation::impl_segment() const
	{
		return _left.segment().combined_right(_right.segment());
	}

	PatternBinaryOperator PatternBinaryOperation::op() const
	{
		return pattern_binary_operator(_op.type);
	}
	
	PatternLazy::PatternLazy(Token&& lazy_token, PatternExpression& expression)
		: _lazy_token(std::move(lazy_token)), _expression(expression)
	{
	}

	void PatternLazy::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternLazy::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternLazy::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment PatternLazy::impl_segment() const
	{
		return _lazy_token.segment.combined_right(_expression.segment());
	}

	PatternCapture::PatternCapture(Token&& capture_token, Token&& identifier, PatternExpression& expression)
		: _capture_token(std::move(capture_token)), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void PatternCapture::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternCapture::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void PatternCapture::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	ScriptSegment PatternCapture::impl_segment() const
	{
		return _capture_token.segment.combined_right(_expression.segment());
	}

	AppendStatement::AppendStatement(Token&& append_token, PatternExpression& expression)
		: _append_token(std::move(append_token)), _expression(expression)
	{
	}

	void AppendStatement::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void AppendStatement::post_analyse(ResolutionContext& ctx)
	{
		// TODO
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

	FindStatement::FindStatement(Token&& find_token, Token&& identifier)
		: _find_token(std::move(find_token)), _identifier(std::move(identifier))
	{
	}

	void FindStatement::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void FindStatement::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment FindStatement::impl_segment() const
	{
		return _find_token.segment.combined_right(_identifier.segment);
	}

	FilterStatement::FilterStatement(Token&& filter_token, Token&& identifier)
		: _filter_token(std::move(filter_token)), _identifier(std::move(identifier))
	{
	}

	void FilterStatement::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void FilterStatement::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment FilterStatement::impl_segment() const
	{
		return _filter_token.segment;
	}

	ReplaceStatement::ReplaceStatement(Token&& replace_token, Expression& match, Expression& string)
		: _replace_token(std::move(replace_token)), _match(match), _string(string)
	{
	}

	void ReplaceStatement::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void ReplaceStatement::post_analyse(ResolutionContext& ctx)
	{
		// TODO
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

	void ApplyStatement::pre_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	void ApplyStatement::post_analyse(ResolutionContext& ctx)
	{
		// TODO
	}

	ScriptSegment ApplyStatement::impl_segment() const
	{
		return _apply_token.segment;
	}

	ScopeStatement::ScopeStatement(Token&& scope_token, Token&& symbol_token, BuiltinSymbol specifier, Expression& range)
		: _scope_token(std::move(scope_token)), _symbol_token(std::move(symbol_token)), _specifier(specifier), _range(range)
	{
	}

	void ScopeStatement::pre_analyse(ResolutionContext& ctx)
	{
		if (data_type(_specifier) == DataType::_Scope)
			_validated = true;
		else
			ctx.add_semantic_error(_symbol_token.segment, "unrecognized scope symbol");
	}

	void ScopeStatement::post_analyse(ResolutionContext& ctx)
	{
		const auto range_type = _range.evaltype(ctx);
		if (!can_cast(range_type, DataType::IRange))
			ctx.add_semantic_error(_range.segment(), "cannot convert to 'irange'");
	}

	void ScopeStatement::traverse(ASTVisitor& visitor)
	{
		ASTNode::traverse(visitor);
		_range.accept(visitor);
	}

	ScriptSegment ScopeStatement::impl_segment() const
	{
		return _scope_token.segment;
	}

	PagePush::PagePush(Token&& page_token, Expression& page)
		: _page_token(std::move(page_token)), _page(page)
	{
	}

	void PagePush::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void PagePush::post_analyse(ResolutionContext& ctx)
	{
		if (!is_pageable(_page.evaltype(ctx)))
		{
			std::stringstream ss;
			ss << "cannot push " << _page.evaltype(ctx) << " to page stack";
			ctx.add_semantic_error(_page.segment(), ss.str());
		}
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

	void PagePop::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void PagePop::post_analyse(ResolutionContext& ctx)
	{
		// NOP
	}

	ScriptSegment PagePop::impl_segment() const
	{
		return _page_token.segment;
	}

	PageClearStack::PageClearStack(Token&& page_token)
		: _page_token(std::move(page_token))
	{
	}

	void PageClearStack::pre_analyse(ResolutionContext& ctx)
	{
		_validated = true;
	}

	void PageClearStack::post_analyse(ResolutionContext& ctx)
	{
		// NOP
	}

	ScriptSegment PageClearStack::impl_segment() const
	{
		return _page_token.segment;
	}
}
