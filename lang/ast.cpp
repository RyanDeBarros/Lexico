#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	ASTNode& AbstractSyntaxTree::add(std::unique_ptr<ASTNode>&& node)
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
		: _global(global), _identifier(std::move(identifier)), _expression(&expression)
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

	VariableAssignment::VariableAssignment(const Token& identifier, Expression& expression)
		: _identifier(identifier), _expression(&expression)
	{
	}

	LiteralExpression::LiteralExpression(const Token& literal)
		: _literal(literal)
	{
	}

	BinaryExpression::BinaryExpression(const Token& op, Expression& left, Expression& right)
		: _op(op), _left(&left), _right(&right)
	{
	}

	VariableExpression::VariableExpression(const Token& identifier)
		: _identifier(identifier)
	{
	}

	BuiltinSymbolExpression::BuiltinSymbolExpression(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}

	FunctionCallExpression::FunctionCallExpression(const Token& identifier, std::vector<Expression*>&& args)
		: _identifier(identifier), _args(std::move(args))
	{
	}

	FunctionDefinition::FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type)
		: _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	ReturnStatement::ReturnStatement(Expression& expression)
		: _expression(&expression)
	{
	}
	
	IfStatement::IfStatement(Expression& condition)
		: _condition(&condition)
	{
	}
	
	ElifStatement::ElifStatement(Expression& condition)
		: _condition(&condition)
	{
	}
	
	WhileLoopStatement::WhileLoopStatement(Expression& condition)
		: _condition(&condition)
	{
	}
	
	ForLoopStatement::ForLoopStatement(Token&& iterator, Expression& iterable)
		: _iterator(std::move(iterator)), _iterable(&iterable)
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

	PatternLiteral::PatternLiteral(const Token& literal)
		: _literal(literal)
	{
	}

	PatternIdentifier::PatternIdentifier(const Token& identifier)
		: _identifier(identifier)
	{
	}

	PatternBuiltin::PatternBuiltin(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}
	
	PatternAs::PatternAs(PatternExpression& expression, const Token& type)
		: _expression(&expression), _type(type)
	{
	}
	
	PatternRepeat::PatternRepeat(PatternExpression& expression, Expression& range)
		: _expression(&expression), _range(&range)
	{
	}
	
	PatternSimpleRepeat::PatternSimpleRepeat(PatternExpression& expression, const Token& op)
		: _expression(&expression), _op(op)
	{
	}
	
	PatternPrefixOperation::PatternPrefixOperation(const Token& op, PatternExpression& expression)
		: _op(op), _expression(&expression)
	{
	}
	
	PatternBackRef::PatternBackRef(const Token& identifier)
		: _identifier(identifier)
	{
	}
	
	PatternBinaryOperation::PatternBinaryOperation(const Token& op, PatternExpression& left, PatternExpression& right)
		: _op(op), _left(&left), _right(&right)
	{
	}
	
	PatternLazy::PatternLazy(PatternExpression& expression)
		: _expression(&expression)
	{
	}
	
	PatternCapture::PatternCapture(const Token& identifier, PatternExpression& expression)
		: _identifier(identifier), _expression(&expression)
	{
	}

	AppendStatement::AppendStatement(PatternExpression& expression)
		: _expression(&expression)
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
		: _match(&match), _string(&string)
	{
	}
	
	ApplyStatement::ApplyStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}
	
	ScopeStatement::ScopeStatement(Token&& specifier, Expression& range)
		: _specifier(std::move(specifier)), _range(&range)
	{
	}
	
	PagePush::PagePush(Expression& page)
		: _page(&page)
	{
	}
}
