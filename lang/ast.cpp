#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	void ASTNode::accept(ASTVisitor& visitor) const
	{
		visitor.visit(*this);
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

	void Block::append(ASTNode& child)
	{
		_children.push_back(&child);
	}

	void Block::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		for (const ASTNode* node : _children)
			node->accept(visitor);
	}

	VariableDeclaration::VariableDeclaration(bool global, Token&& identifier, Expression& expression)
		: _global(global), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableDeclaration::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
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

	VariableAssignment::VariableAssignment(Token&& identifier, Expression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableAssignment::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	BinaryExpression::BinaryExpression(Token&& op, Expression& left, Expression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void BinaryExpression::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}

	PrefixExpression::PrefixExpression(Token&& op, Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	void PrefixExpression::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expr.accept(visitor);
	}

	AsExpression::AsExpression(Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expr.accept(visitor);
	}

	SubscriptExpression::SubscriptExpression(Expression& container, Expression& subscript)
		: _container(container), _subscript(subscript)
	{
	}

	void SubscriptExpression::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_container.accept(visitor);
		_subscript.accept(visitor);
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	BuiltinSymbolExpression::BuiltinSymbolExpression(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}

	FunctionCallExpression::FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args)
		: _identifier(std::move(identifier)), _args(std::move(args))
	{
	}

	void FunctionCallExpression::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		for (const Expression* arg : _args)
			arg->accept(visitor);
	}

	FunctionDefinition::FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type)
		: _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	ReturnStatement::ReturnStatement(Expression* expression)
		: _expression(expression)
	{
	}
	
	void ReturnStatement::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		if (_expression)
			_expression->accept(visitor);
	}
	
	IfStatement::IfStatement(Expression& condition)
		: _condition(condition)
	{
	}

	void IfStatement::accept(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::accept(visitor);
	}
	
	ElifStatement::ElifStatement(Expression& condition)
		: _condition(condition)
	{
	}

	void ElifStatement::accept(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::accept(visitor);
	}
	
	WhileLoop::WhileLoop(Expression& condition)
		: _condition(condition)
	{
	}

	void WhileLoop::accept(ASTVisitor& visitor) const
	{
		_condition.accept(visitor);
		Block::accept(visitor);
	}

	ForLoop::ForLoop(Token&& iterator, Expression& iterable)
		: _iterator(std::move(iterator)), _iterable(iterable)
	{
	}

	void ForLoop::accept(ASTVisitor& visitor) const
	{
		_iterable.accept(visitor);
		Block::accept(visitor);
	}

	LogStatement::LogStatement(std::vector<Expression*>&& args)
		: _args(std::move(args))
	{
	}
	
	HighlightStatement::HighlightStatement(bool clear, Expression* highlightable, BuiltinSymbol color)
		: _clear(clear), _highlightable(highlightable), _color(color)
	{
	}

	void HighlightStatement::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		if (_highlightable)
			_highlightable->accept(visitor);
	}

	DeletePattern::DeletePattern(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	PatternDeclaration::PatternDeclaration(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	PatternSubexpression::PatternSubexpression(Expression& expr)
		: _expr(expr)
	{
	}

	void PatternSubexpression::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expr.accept(visitor);
	}

	PatternLiteral::PatternLiteral(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	PatternIdentifier::PatternIdentifier(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	PatternBuiltin::PatternBuiltin(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}
	
	PatternAs::PatternAs(PatternExpression& expression, Token&& type)
		: _expression(expression), _type(std::move(type))
	{
	}

	void PatternAs::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}

	PatternRepeat::PatternRepeat(PatternExpression& expression, Expression& range)
		: _expression(expression), _range(range)
	{
	}

	void PatternRepeat::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
		_range.accept(visitor);
	}
	
	PatternSimpleRepeat::PatternSimpleRepeat(PatternExpression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}

	void PatternSimpleRepeat::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}
	
	PatternPrefixOperation::PatternPrefixOperation(Token&& op, PatternExpression& expression)
		: _op(std::move(op)), _expression(expression)
	{
	}

	void PatternPrefixOperation::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}
	
	PatternBackRef::PatternBackRef(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	PatternBinaryOperation::PatternBinaryOperation(Token&& op, PatternExpression& left, PatternExpression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void PatternBinaryOperation::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_left.accept(visitor);
		_right.accept(visitor);
	}
	
	PatternLazy::PatternLazy(PatternExpression& expression)
		: _expression(expression)
	{
	}

	void PatternLazy::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}
	
	PatternCapture::PatternCapture(Token&& identifier, PatternExpression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void PatternCapture::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}

	AppendStatement::AppendStatement(PatternExpression& expression)
		: _expression(expression)
	{
	}

	void AppendStatement::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_expression.accept(visitor);
	}

	FindStatement::FindStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	FilterStatement::FilterStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	ReplaceStatement::ReplaceStatement(Expression& match, Expression& string)
		: _match(match), _string(string)
	{
	}

	void ReplaceStatement::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_match.accept(visitor);
		_string.accept(visitor);
	}
	
	ApplyStatement::ApplyStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	ScopeStatement::ScopeStatement(Token&& specifier, Expression& range)
		: _specifier(std::move(specifier)), _range(range)
	{
	}

	void ScopeStatement::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_range.accept(visitor);
	}
	
	PagePush::PagePush(Expression& page)
		: _page(page)
	{
	}

	void PagePush::accept(ASTVisitor& visitor) const
	{
		ASTNode::accept(visitor);
		_page.accept(visitor);
	}
}
