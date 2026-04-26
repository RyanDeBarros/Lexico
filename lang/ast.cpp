#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
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

	VariableDeclaration::VariableDeclaration(bool global, Token&& identifier, Expression& expression)
		: _global(global), _identifier(std::move(identifier)), _expression(expression)
	{
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

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	BinaryExpression::BinaryExpression(Token&& op, Expression& left, Expression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	PrefixExpression::PrefixExpression(Token&& op, Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	AsExpression::AsExpression(Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	SubscriptExpression::SubscriptExpression(Expression& container, Expression& subscript)
		: _container(container), _subscript(subscript)
	{
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

	FunctionDefinition::FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type)
		: _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	ReturnStatement::ReturnStatement(Expression* expression)
		: _expression(expression)
	{
	}
	
	IfStatement::IfStatement(Expression& condition)
		: _condition(condition)
	{
	}
	
	ElifStatement::ElifStatement(Expression& condition)
		: _condition(condition)
	{
	}
	
	WhileLoop::WhileLoop(Expression& condition)
		: _condition(condition)
	{
	}
	
	ForLoop::ForLoop(Token&& iterator, Expression& iterable)
		: _iterator(std::move(iterator)), _iterable(iterable)
	{
	}
	
	LogStatement::LogStatement(std::vector<Expression*>&& args)
		: _args(std::move(args))
	{
	}
	
	HighlightStatement::HighlightStatement(bool clear, Expression* highlightable, BuiltinSymbol color)
		: _clear(clear), _highlightable(highlightable), _color(color)
	{
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
	
	PatternRepeat::PatternRepeat(PatternExpression& expression, Expression& range)
		: _expression(expression), _range(range)
	{
	}
	
	PatternSimpleRepeat::PatternSimpleRepeat(PatternExpression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}
	
	PatternPrefixOperation::PatternPrefixOperation(Token&& op, PatternExpression& expression)
		: _op(std::move(op)), _expression(expression)
	{
	}
	
	PatternBackRef::PatternBackRef(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	PatternBinaryOperation::PatternBinaryOperation(Token&& op, PatternExpression& left, PatternExpression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}
	
	PatternLazy::PatternLazy(PatternExpression& expression)
		: _expression(expression)
	{
	}
	
	PatternCapture::PatternCapture(Token&& identifier, PatternExpression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	AppendStatement::AppendStatement(PatternExpression& expression)
		: _expression(expression)
	{
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
	
	ApplyStatement::ApplyStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	ScopeStatement::ScopeStatement(Token&& specifier, Expression& range)
		: _specifier(std::move(specifier)), _range(range)
	{
	}
	
	PagePush::PagePush(Expression& page)
		: _page(page)
	{
	}
}
