#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	void ASTNode::accept(ASTVisitor& visitor) const
	{
		visitor.pre_visit(*this);
		traverse(visitor);
		visitor.post_visit(*this);
	}

	ASTNode& AbstractSyntaxTree::add_impl(std::unique_ptr<ASTNode>&& node)
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
		if (!validated)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": node is not validated";
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
		if (auto line_number = env.identifier_is_registered(_identifier.lexeme, _global ? Namespace::Unknown : Namespace::Isolated))
		{
			validated = false;
			env.errors().push_back(LxError::token_error(_identifier, env.script_lines(), ErrorType::Semantic, "identifier already declared on line " + std::to_string(*line_number)));
		}
		else
			validated = true;
	}

	void VariableDeclaration::post_analyse(RuntimeEnvironment& env) const
	{
		if (!validated)
			return;

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

	std::string_view VariableDeclaration::variable_name() const
	{
		return _identifier.lexeme;
	}

	VariableAssignment::VariableAssignment(Token&& identifier, const Expression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableAssignment::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void VariableAssignment::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
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
		validated = true;
	}

	void LiteralExpression::post_analyse(RuntimeEnvironment& env) const
	{
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
		// TODO
	}

	void BinaryExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void BinaryExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	DataType BinaryExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO switch over operator and combo of _left.evaltype() and _right.evaltype()
		return DataType::Void;
	}

	PrefixExpression::PrefixExpression(Token&& op, const Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	void PrefixExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PrefixExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void PrefixExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType PrefixExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO switch over operator and _expr.evaltype()
		return DataType::Void;
	}

	AsExpression::AsExpression(const Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void AsExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void AsExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.accept(visitor);
	}

	DataType AsExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO refactor static_cast<DataType>(TokenType) into safer utility that will check that TokenType is indeed a valid data type
		return static_cast<DataType>(_type.type);
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
		// TODO
	}

	void VariableExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	DataType VariableExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		if (auto sig = env.variable_is_registered(_identifier.lexeme, Namespace::Unknown))
			return sig->type;
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
	}

	DataType BuiltinSymbolExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		// TODO create new symbol types?
		return DataType::Void;
	}

	FunctionCallExpression::FunctionCallExpression(Token&& identifier, std::vector<const Expression*>&& args)
		: _identifier(std::move(identifier)), _args(std::move(args))
	{
	}

	void FunctionCallExpression::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void FunctionCallExpression::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void FunctionCallExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		for (const Expression* arg : _args)
			arg->accept(visitor);
	}

	DataType FunctionCallExpression::impl_evaltype(const RuntimeEnvironment& env) const
	{
		if (auto sig = env.function_is_registered(_identifier.lexeme, Namespace::Unknown))
			return sig->return_type;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function is not registered";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	FunctionDefinition::FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type)
		: _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	void FunctionDefinition::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void FunctionDefinition::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	bool FunctionDefinition::isolated() const
	{
		return true;
	}

	ReturnStatement::ReturnStatement(const Expression* expression)
		: _expression(expression)
	{
	}

	void ReturnStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ReturnStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO
	}

	void ReturnStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		if (_expression)
			_expression->accept(visitor);
	}
	
	IfStatement::IfStatement(const Expression& condition)
		: _condition(condition)
	{
	}

	void IfStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);

		// TODO
	}

	void IfStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

		Block::post_analyse(env);
	}

	void IfStatement::traverse(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
	}

	bool IfStatement::isolated() const
	{
		return false;
	}
	
	ElifStatement::ElifStatement(const Expression& condition)
		: _condition(condition)
	{
	}

	void ElifStatement::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);

		// TODO
	}

	void ElifStatement::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

		Block::post_analyse(env);
	}

	void ElifStatement::traverse(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::traverse(visitor);
	}

	bool ElifStatement::isolated() const
	{
		return false;
	}

	bool ElseStatement::isolated() const
	{
		return false;
	}
	
	WhileLoop::WhileLoop(const Expression& condition)
		: _condition(condition)
	{
	}

	void WhileLoop::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);

		// TODO
	}

	void WhileLoop::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

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

	ForLoop::ForLoop(Token&& iterator, const Expression& iterable)
		: _iterator(std::move(iterator)), _iterable(iterable)
	{
	}

	void ForLoop::pre_analyse(RuntimeEnvironment& env) const
	{
		Block::pre_analyse(env);

		// TODO
	}

	void ForLoop::post_analyse(RuntimeEnvironment& env) const
	{
		// TODO

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
