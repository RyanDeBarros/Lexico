#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	bool ASTNode::validated() const
	{
		return _validated;
	}

	void ASTNode::accept(ASTVisitor& visitor) const
	{
		visitor.pre_visit(*this);
		traverse(visitor);
		visitor.post_visit(*this);
	}

	UpflowInfo ASTNode::upflow() const
	{
		if (!_upflow)
			_upflow = impl_upflow();
		return *_upflow;
	}

	UpflowInfo ASTNode::impl_upflow() const
	{
		return {};
	}

	ASTNode& AbstractSyntaxTree::impl_add(std::unique_ptr<ASTNode>&& node)
	{
		ASTNode* ref = node.get();
		_nodes.push_back(std::move(node));
		return *ref;
	}

	const ASTRoot& AbstractSyntaxTree::root() const
	{
		return _root;
	}

	ASTRoot& AbstractSyntaxTree::root()
	{
		return _root;
	}

	void Block::pre_analyse(RuntimeEnvironment& env) const
	{
		env.push_local_scope(isolated());
	}

	void Block::post_analyse(RuntimeEnvironment& env) const
	{
		env.pop_local_scope();
	}

	void Block::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		for (const ASTNode* node : _children)
			node->accept(visitor);
	}

	UpflowInfo Block::impl_upflow() const
	{
		UpflowInfo info;

		for (const ASTNode* node : _children)
		{
			auto subinfo = node->upflow();
			if (subinfo.all_return)
			{
				info.all_return = true;
				info.returns.insert(info.returns.end(), subinfo.returns.begin(), subinfo.returns.end());
			}
		}

		return info;
	}

	void Block::append(ASTNode& child)
	{
		_children.push_back(&child);
	}

	bool ASTRoot::isolated() const
	{
		return true;
	}

	DataType Expression::evaltype(const RuntimeEnvironment& env) const
	{
		if (!_validated)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": node is not _validated";
			throw LxError(ErrorType::Internal, ss.str());
		}
		if (!_evaltype)
			_evaltype = impl_evaltype(env);
		return *_evaltype;
	}

	VariableDeclaration::VariableDeclaration(bool global, Token&& identifier, const Expression& expression)
		: _global(global), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableDeclaration::pre_analyse(RuntimeEnvironment& env) const
	{
		if (_global && env.scope_depth() > 0)
			env.add_semantic_error(_identifier, "cannot declare global variable inside a scope");
		else if (auto ln = env.identifier_first_decl_line_number(_identifier.lexeme, _global ? Namespace::Unknown : Namespace::Local))
			env.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(*ln));
		else
			_validated = true;
	}

	void VariableDeclaration::post_analyse(RuntimeEnvironment& env) const
	{
		env.register_variable(_identifier.lexeme, _expression.evaltype(env), _identifier.start_line, _global ? Namespace::Global : Namespace::Local);
	}

	void VariableDeclaration::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	bool VariableDeclaration::is_global() const
	{
		return _global;
	}

	VariableAssignment::VariableAssignment(Token&& identifier, const Expression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableAssignment::pre_analyse(RuntimeEnvironment& env) const
	{
		if (auto var = env.registered_variable(_identifier.lexeme, Namespace::Unknown))
			_validated = true;
		else
			env.add_semantic_error(_identifier, "variable is not declared in scope");
	}

	void VariableAssignment::post_analyse(RuntimeEnvironment& env) const
	{
		auto var = env.registered_variable(_identifier.lexeme, Namespace::Unknown);
		if (var->type != _expression.evaltype(env))
		{
			std::stringstream ss;
			ss << "cannot assign variable of type '" << friendly_name(var->type) << "' to expression of type '" << friendly_name(_expression.evaltype(env)) << "'";
			env.add_semantic_error(_identifier, ss.str());
		}
	}

	void VariableAssignment::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void LiteralExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		_validated = true;
	}

	void LiteralExpression::post_analyse(RuntimeEnvironment& env) const
	{
		evaltype(env);
	}

	DataType LiteralExpression::impl_evaltype(const RuntimeEnvironment& env) const
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

	BinaryExpression::BinaryExpression(Token&& op, const Expression& left, const Expression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void BinaryExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		_validated = true;
	}

	void BinaryExpression::post_analyse(RuntimeEnvironment& env) const
	{
		evaltype(env);
	}

	void BinaryExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	DataType BinaryExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		if (auto type = lx::evaltype(op(), _left.evaltype(env), _right.evaltype(env)))
			return *type;
		else
		{
			std::stringstream ss;
			ss << ": operator not defined for types '" << friendly_name(_left.evaltype(env)) << "' and '" << friendly_name(_right.evaltype(env)) << "'";
			env.add_semantic_error(_op, ss.str());
			return DataType::Void;
		}
	}

	StandardBinaryOperator BinaryExpression::op() const
	{
		return standard_binary_operator(_op.type);
	}

	MemberAccessExpression::MemberAccessExpression(const Expression& object, Token&& member)
		: _object(object), _member(std::move(member))
	{
	}

	void MemberAccessExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		_validated = true;
	}

	void MemberAccessExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO validate that member exists for object.evaltype()

		evaltype(env);
	}

	void MemberAccessExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_object.accept(visitor);
	}

	DataType MemberAccessExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO look up class table for member
		return DataType::Void;
	}

	PrefixExpression::PrefixExpression(Token&& op, const Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	void PrefixExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		_validated = true;
	}

	void PrefixExpression::post_analyse(RuntimeEnvironment& env) const
	{
		evaltype(env);
	}

	void PrefixExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType PrefixExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		if (auto type = lx::evaltype(op(), _expr.evaltype(env)))
			return *type;
		else
		{
			std::stringstream ss;
			ss << ": operator not defined for type '" << friendly_name(_expr.evaltype(env)) << "'";
			env.add_semantic_error(_op, ss.str());
			return DataType::Void;
		}
	}

	StandardPrefixOperator PrefixExpression::op() const
	{
		return standard_prefix_operator(_op.type);
	}

	AsExpression::AsExpression(const Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		_validated = true;
	}

	void AsExpression::post_analyse(RuntimeEnvironment& env) const
	{
		evaltype(env);
	}

	void AsExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType AsExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		DataType return_type = data_type(_type.type);
		if (can_cast(_expr.evaltype(env), return_type))
			return return_type;
		else
		{
			std::stringstream ss;
			ss << ": cannot convert from '" << friendly_name(_expr.evaltype(env)) << "' to '" << friendly_name(return_type) << "'";
			env.add_semantic_error(_type, ss.str());
			return DataType::Void;
		}
	}

	SubscriptExpression::SubscriptExpression(const Expression& container, const Expression& subscript)
		: _container(container), _subscript(subscript)
	{
	}

	void SubscriptExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void SubscriptExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

		evaltype(env);
	}

	void SubscriptExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_container.accept(visitor);
		_subscript.accept(visitor);
	}

	DataType SubscriptExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO switch over evaltype() of _container to see what [] method should return
		return DataType::Void;
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void VariableExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		if (env.registered_variable(_identifier.lexeme, Namespace::Unknown))
			_validated = true;
		else
			env.add_semantic_error(_identifier, "variable is not declared in scope");
	}

	void VariableExpression::post_analyse(RuntimeEnvironment& env) const
	{
		evaltype(env);
	}

	DataType VariableExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		if (auto var = env.registered_variable(_identifier.lexeme, Namespace::Unknown))
			return var->type;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable is not registered";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	BuiltinSymbolExpression::BuiltinSymbolExpression(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}

	void BuiltinSymbolExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void BuiltinSymbolExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

		evaltype(env);
	}

	DataType BuiltinSymbolExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO create new internal symbol types then switch over symbol enum
		return DataType::Void;
	}

	FunctionCallExpression::FunctionCallExpression(Token&& identifier, std::vector<const Expression*>&& args)
		: _identifier(std::move(identifier)), _args(std::move(args))
	{
	}

	void FunctionCallExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		if (env.registered_function_calls(_identifier.lexeme, Namespace::Unknown).empty())
			env.add_semantic_error(_identifier, "function not declared in scope");
		else
			_validated = true;
	}

	void FunctionCallExpression::post_analyse(RuntimeEnvironment& env) const
	{
		auto argtypes = arg_types(env);
		if (!env.registered_function(_identifier.lexeme, argtypes, Namespace::Unknown))
		{
			std::stringstream ss;
			ss << "no declaration of '" << _identifier.lexeme << "' matches the arguemnt types (";
			for (size_t i = 0; i < argtypes.size(); ++i)
			{
				ss << "'" << friendly_name(argtypes[i]) << "'";
				if (i + 1 < argtypes.size())
					ss << ", ";
			}
			ss << ")";
			env.add_semantic_error(_identifier, ss.str());
		}
	}

	void FunctionCallExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		for (const Expression* arg : _args)
			arg->accept(visitor);
	}

	DataType FunctionCallExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		if (auto fn = env.registered_function(_identifier.lexeme, arg_types(env), Namespace::Unknown))
			return fn->return_type;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function is not registered";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	std::vector<DataType> FunctionCallExpression::arg_types(const RuntimeEnvironment& env) const
	{
		std::vector<DataType> args(_args.size());
		for (size_t i = 0; i < args.size(); ++i)
			args[i] = _args[i]->evaltype(env);
		return args;
	}

	MethodCallExpression::MethodCallExpression(const Expression& object, std::vector<const Expression*>&& args)
		: _object(object), _args(std::move(args))
	{
	}

	void MethodCallExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void MethodCallExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

		evaltype(env);
	}

	void MethodCallExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		for (const Expression* arg : _args)
			arg->accept(visitor);
	}

	DataType MethodCallExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO
		return DataType::Void;
	}

	FunctionDefinition::FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, std::optional<Token>&& return_type)
		: _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	void FunctionDefinition::pre_analyse(RuntimeEnvironment& env) const
	{
		if (auto var = env.registered_variable(_identifier.lexeme, Namespace::Unknown))
		{
			env.add_semantic_error(_identifier, "identifier already declared on line " + std::to_string(var->decl_line_number));
			return;
		}
		
		if (auto fn = env.registered_function(_identifier.lexeme, arg_types(), Namespace::Unknown))
		{
			env.add_semantic_error(_identifier, "function with matching argument types already declared on line " + std::to_string(fn->decl_line_number));
			return;
		}

		std::unordered_set<std::string_view> argnames;
		for (const auto& [_, identifier] : _arglist)
		{
			if (argnames.count(identifier.lexeme))
				env.add_semantic_error(identifier, "repeated argument identifier");
			else
				argnames.insert(identifier.lexeme);
		}

		if (argnames.size() != _arglist.size())
			return;

		env.register_function(_identifier.lexeme, return_type(), arg_types(), _identifier.start_line, env.scope_depth() == 0 ? Namespace::Global : Namespace::Local);

		Block::pre_analyse(env);

		for (size_t i = 0; i < _arglist.size(); ++i)
			env.register_variable(_arglist[i].second.lexeme, data_type(_arglist[i].first.type), _arglist[i].second.start_line, Namespace::Local);

		_validated = true;
	}

	void FunctionDefinition::post_analyse(RuntimeEnvironment& env) const
	{
		auto flow = Block::impl_upflow();

		if (return_type() != DataType::Void && !flow.all_return)
			env.add_semantic_error(_identifier, "not all control paths return a value");

		for (const ReturnStatement* r : flow.returns)
		{
			if (r->evaltype(env) != return_type())
			{
				std::stringstream ss;
				ss << "function should return '" << friendly_name(return_type()) << "' but statement returns '" << friendly_name(r->evaltype(env)) << "'";
				env.add_semantic_error(r->return_token(), ss.str());
			}
		}

		Block::post_analyse(env);
	}

	UpflowInfo FunctionDefinition::impl_upflow() const
	{
		return {};
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

	ReturnStatement::ReturnStatement(Token&& return_token, const Expression* expression)
		: _return_token(std::move(return_token)), _expression(expression)
	{
	}

	void ReturnStatement::pre_analyse(RuntimeEnvironment& env) const
	{
	}

	void ReturnStatement::post_analyse(RuntimeEnvironment& env) const
	{
	}

	void ReturnStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		if (_expression)
			_expression->accept(visitor);
	}

	UpflowInfo ReturnStatement::impl_upflow() const
	{
		return { .all_return = true, .returns = { this } };
	}

	DataType ReturnStatement::evaltype(const RuntimeEnvironment& env) const
	{
		return _expression ? _expression->evaltype(env) : DataType::Void;
	}

	const Token& ReturnStatement::return_token() const
	{
		return _return_token;
	}

	void IfConditional::set_fallback(const IfFallbackBlock* fallback)
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

	UpflowInfo IfConditional::impl_upflow() const
	{
		auto info = Block::impl_upflow();
		if (_fallback)
		{
			auto fallback_info = _fallback->upflow();
			if (!fallback_info.all_return)
				info.all_return = false;

			info.returns.insert(info.returns.end(), fallback_info.returns.begin(), fallback_info.returns.end());
		}
		return info;
	}

	IfStatement::IfStatement(Token&& if_token, const Expression& condition)
		: _if_token(std::move(if_token)), _condition(condition)
	{
	}

	void IfStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);
		_validated = true;
	}

	void IfStatement::post_analyse(RuntimeEnvironment& env) const
	{
		if (_condition.evaltype(env) != DataType::Bool)
			env.add_semantic_error(_if_token, "condition expression does not resolve to a 'bool'");
		Block::post_analyse(env);
	}

	void IfStatement::traverse(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
		if (_fallback)
			_fallback->accept(visitor);
	}

	UpflowInfo IfStatement::impl_upflow() const
	{
		return IfConditional::impl_upflow();
	}

	bool IfStatement::isolated() const
	{
		return false;
	}

	ElifStatement::ElifStatement(Token&& elif_token, const Expression& condition)
		: _elif_token(std::move(elif_token)), _condition(condition)
	{
	}

	void ElifStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);
		_validated = true;
	}

	void ElifStatement::post_analyse(RuntimeEnvironment& env) const
	{
		if (_condition.evaltype(env) != DataType::Bool)
			env.add_semantic_error(_elif_token, "condition expression does not resolve to a 'bool'");
		Block::post_analyse(env);
	}

	void ElifStatement::traverse(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
		if (_fallback)
			_fallback->accept(visitor);
	}

	UpflowInfo ElifStatement::impl_upflow() const
	{
		return IfConditional::impl_upflow();
	}

	bool ElifStatement::isolated() const
	{
		return false;
	}

	bool ElseStatement::isolated() const
	{
		return false;
	}
	
	WhileLoop::WhileLoop(Token&& while_token, const Expression& condition)
		: _while_token(std::move(while_token)), _condition(condition)
	{
	}

	void WhileLoop::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);
		_validated = true;
	}

	void WhileLoop::post_analyse(RuntimeEnvironment& env) const
	{
		if (_condition.evaltype(env) != DataType::Bool)
			env.add_semantic_error(_while_token, "condition expression does not resolve to a 'bool'");
		Block::post_analyse(env);
	}

	void WhileLoop::traverse(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
	}

	bool WhileLoop::isolated() const
	{
		return false;
	}

	ForLoop::ForLoop(Token&& for_token, Token&& iterator, const Expression& iterable)
		: _for_token(std::move(for_token)), _iterator(std::move(iterator)), _iterable(iterable)
	{
	}

	void ForLoop::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);
		env.registered_variable(_iterator.lexeme, Namespace::Local);
		_validated = true;
	}

	void ForLoop::post_analyse(RuntimeEnvironment& env) const
	{
		if (!is_iterable(_iterable.evaltype(env)))
		{
			std::stringstream ss;
			ss << "'" << friendly_name(_iterable.evaltype(env)) << "' is not iterable";
			env.add_semantic_error(_for_token, ss.str());
		}
		Block::post_analyse(env);
	}

	void ForLoop::traverse(ASTVisitor& visitor) const
	{
		_iterable.accept(visitor);
		Block::traverse(visitor);
	}

	bool ForLoop::isolated() const
	{
		return false;
	}

	LogStatement::LogStatement(std::vector<const Expression*>&& args)
		: _args(std::move(args))
	{
	}
	
	HighlightStatement::HighlightStatement(bool clear, const Expression* highlightable, BuiltinSymbol color)
		: _clear(clear), _highlightable(highlightable), _color(color)
	{
	}

	void HighlightStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void HighlightStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void HighlightStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		if (_highlightable)
			_highlightable->accept(visitor);
	}

	DeletePattern::DeletePattern(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void DeletePattern::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void DeletePattern::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	PatternDeclaration::PatternDeclaration(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternDeclaration::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternDeclaration::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	PatternSubexpression::PatternSubexpression(const Expression& expr)
		: _expr(expr)
	{
	}

	void PatternSubexpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternSubexpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternSubexpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	PatternLiteral::PatternLiteral(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void PatternLiteral::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternLiteral::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	PatternIdentifier::PatternIdentifier(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternIdentifier::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternIdentifier::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	PatternBuiltin::PatternBuiltin(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}

	void PatternBuiltin::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternBuiltin::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	PatternAs::PatternAs(const PatternExpression& expression, Token&& type)
		: _expression(expression), _type(std::move(type))
	{
	}

	void PatternAs::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternAs::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternAs::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	PatternRepeat::PatternRepeat(const PatternExpression& expression, const Expression& range)
		: _expression(expression), _range(range)
	{
	}

	void PatternRepeat::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternRepeat::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternRepeat::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
		_range.accept(visitor);
	}
	
	PatternSimpleRepeat::PatternSimpleRepeat(const PatternExpression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}

	void PatternSimpleRepeat::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternSimpleRepeat::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternSimpleRepeat::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	PatternSimpleRepeatOperator PatternSimpleRepeat::op() const
	{
		return pattern_simple_repeat_operator(_op.type);
	}

	PatternPrefixOperation::PatternPrefixOperation(Token&& op, const PatternExpression& expression)
		: _op(std::move(op)), _expression(expression)
	{
	}

	void PatternPrefixOperation::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternPrefixOperation::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternPrefixOperation::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	PatternPrefixOperator PatternPrefixOperation::op() const
	{
		return pattern_prefix_operator(_op.type);
	}
	
	PatternBackRef::PatternBackRef(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternBackRef::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternBackRef::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	PatternBinaryOperation::PatternBinaryOperation(Token&& op, const PatternExpression& left, const PatternExpression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void PatternBinaryOperation::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternBinaryOperation::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternBinaryOperation::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	PatternBinaryOperator PatternBinaryOperation::op() const
	{
		return pattern_binary_operator(_op.type);
	}
	
	PatternLazy::PatternLazy(const PatternExpression& expression)
		: _expression(expression)
	{
	}

	void PatternLazy::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternLazy::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternLazy::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}
	
	PatternCapture::PatternCapture(Token&& identifier, const PatternExpression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void PatternCapture::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternCapture::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PatternCapture::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	AppendStatement::AppendStatement(const PatternExpression& expression)
		: _expression(expression)
	{
	}

	void AppendStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void AppendStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void AppendStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.accept(visitor);
	}

	FindStatement::FindStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void FindStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void FindStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	FilterStatement::FilterStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void FilterStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void FilterStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	ReplaceStatement::ReplaceStatement(const Expression& match, const Expression& string)
		: _match(match), _string(string)
	{
	}

	void ReplaceStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ReplaceStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ReplaceStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_match.accept(visitor);
		_string.accept(visitor);
	}
	
	ApplyStatement::ApplyStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void ApplyStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ApplyStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	ScopeStatement::ScopeStatement(Token&& specifier, const Expression& range)
		: _specifier(std::move(specifier)), _range(range)
	{
	}

	void ScopeStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ScopeStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ScopeStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_range.accept(visitor);
	}
	
	PagePush::PagePush(const Expression& page)
		: _page(page)
	{
	}

	void PagePush::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PagePush::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PagePush::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_page.accept(visitor);
	}

	void PagePop::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PagePop::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PageClearStack::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PageClearStack::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}
}
