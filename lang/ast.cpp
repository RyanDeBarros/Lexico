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

	void Block::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		env.push_local_scope(isolated());
	}

	void Block::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		env.pop_local_scope();
	}

	void Block::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		for (const ASTNode* node : _children)
			node->traverse(visitor);
	}

	void Block::append(ASTNode& child)
	{
		_children.push_back(&child);
	}

	bool ASTRoot::isolated() const
	{
		return true;
	}

	VariableDeclaration::VariableDeclaration(bool global, Token&& identifier, Expression& expression)
		: _global(global), _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void VariableDeclaration::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void VariableDeclaration::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void VariableDeclaration::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
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

	void VariableAssignment::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void VariableAssignment::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void VariableAssignment::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}

	LiteralExpression::LiteralExpression(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void LiteralExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void LiteralExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	BinaryExpression::BinaryExpression(Token&& op, Expression& left, Expression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void BinaryExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void BinaryExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void BinaryExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_left.traverse(visitor);
		_right.traverse(visitor);
	}

	PrefixExpression::PrefixExpression(Token&& op, Expression& expr)
		: _op(std::move(op)), _expr(expr)
	{
	}

	void PrefixExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PrefixExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PrefixExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.traverse(visitor);
	}

	AsExpression::AsExpression(Expression& expr, Token&& type)
		: _expr(expr), _type(std::move(type))
	{
	}

	void AsExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void AsExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void AsExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.traverse(visitor);
	}

	SubscriptExpression::SubscriptExpression(Expression& container, Expression& subscript)
		: _container(container), _subscript(subscript)
	{
	}

	void SubscriptExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void SubscriptExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void SubscriptExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_container.traverse(visitor);
		_subscript.traverse(visitor);
	}

	VariableExpression::VariableExpression(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void VariableExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void VariableExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	BuiltinSymbolExpression::BuiltinSymbolExpression(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}

	void BuiltinSymbolExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void BuiltinSymbolExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	FunctionCallExpression::FunctionCallExpression(Token&& identifier, std::vector<Expression*>&& args)
		: _identifier(std::move(identifier)), _args(std::move(args))
	{
	}

	void FunctionCallExpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void FunctionCallExpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void FunctionCallExpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		for (const Expression* arg : _args)
			arg->traverse(visitor);
	}

	FunctionDefinition::FunctionDefinition(Token&& identifier, std::vector<std::pair<Token, Token>>&& arglist, Token&& return_type)
		: _identifier(std::move(identifier)), _arglist(std::move(arglist)), _return_type(std::move(return_type))
	{
	}

	void FunctionDefinition::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void FunctionDefinition::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	bool FunctionDefinition::isolated() const
	{
		return true;
	}

	ReturnStatement::ReturnStatement(Expression* expression)
		: _expression(expression)
	{
	}

	void ReturnStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ReturnStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ReturnStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		if (_expression)
			_expression->traverse(visitor);
	}
	
	IfStatement::IfStatement(Expression& condition)
		: _condition(condition)
	{
	}

	void IfStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		Block::pre_analyse(env, errors);

		// TODO
	}

	void IfStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO

		Block::post_analyse(env, errors);
	}

	void IfStatement::traverse(ASTVisitor& visitor) const
	{
		_condition.traverse(visitor);
		Block::traverse(visitor);
	}

	bool IfStatement::isolated() const
	{
		return false;
	}
	
	ElifStatement::ElifStatement(Expression& condition)
		: _condition(condition)
	{
	}

	void ElifStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		Block::pre_analyse(env, errors);

		// TODO
	}

	void ElifStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO

		Block::post_analyse(env, errors);
	}

	void ElifStatement::traverse(ASTVisitor& visitor) const
	{
		_condition.traverse(visitor);
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
	
	WhileLoop::WhileLoop(Expression& condition)
		: _condition(condition)
	{
	}

	void WhileLoop::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		Block::pre_analyse(env, errors);

		// TODO
	}

	void WhileLoop::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO

		Block::post_analyse(env, errors);
	}

	void WhileLoop::traverse(ASTVisitor& visitor) const
	{
		_condition.traverse(visitor);
		Block::traverse(visitor);
	}

	bool WhileLoop::isolated() const
	{
		return false;
	}

	ForLoop::ForLoop(Token&& iterator, Expression& iterable)
		: _iterator(std::move(iterator)), _iterable(iterable)
	{
	}

	void ForLoop::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		Block::pre_analyse(env, errors);

		// TODO
	}

	void ForLoop::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO

		Block::post_analyse(env, errors);
	}

	void ForLoop::traverse(ASTVisitor& visitor) const
	{
		_iterable.traverse(visitor);
		Block::traverse(visitor);
	}

	bool ForLoop::isolated() const
	{
		return false;
	}

	LogStatement::LogStatement(std::vector<Expression*>&& args)
		: _args(std::move(args))
	{
	}
	
	HighlightStatement::HighlightStatement(bool clear, Expression* highlightable, BuiltinSymbol color)
		: _clear(clear), _highlightable(highlightable), _color(color)
	{
	}

	void HighlightStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void HighlightStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void HighlightStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		if (_highlightable)
			_highlightable->traverse(visitor);
	}

	DeletePattern::DeletePattern(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void DeletePattern::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void DeletePattern::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	PatternDeclaration::PatternDeclaration(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternDeclaration::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternDeclaration::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	PatternSubexpression::PatternSubexpression(Expression& expr)
		: _expr(expr)
	{
	}

	void PatternSubexpression::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternSubexpression::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternSubexpression::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expr.traverse(visitor);
	}

	PatternLiteral::PatternLiteral(Token&& literal)
		: _literal(std::move(literal))
	{
	}

	void PatternLiteral::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternLiteral::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	PatternIdentifier::PatternIdentifier(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternIdentifier::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternIdentifier::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	PatternBuiltin::PatternBuiltin(BuiltinSymbol builtin_symbol)
		: _builtin_symbol(builtin_symbol)
	{
	}

	void PatternBuiltin::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternBuiltin::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	PatternAs::PatternAs(PatternExpression& expression, Token&& type)
		: _expression(expression), _type(std::move(type))
	{
	}

	void PatternAs::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternAs::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternAs::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}

	PatternRepeat::PatternRepeat(PatternExpression& expression, Expression& range)
		: _expression(expression), _range(range)
	{
	}

	void PatternRepeat::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternRepeat::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternRepeat::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
		_range.traverse(visitor);
	}
	
	PatternSimpleRepeat::PatternSimpleRepeat(PatternExpression& expression, Token&& op)
		: _expression(expression), _op(std::move(op))
	{
	}

	void PatternSimpleRepeat::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternSimpleRepeat::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternSimpleRepeat::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}
	
	PatternPrefixOperation::PatternPrefixOperation(Token&& op, PatternExpression& expression)
		: _op(std::move(op)), _expression(expression)
	{
	}

	void PatternPrefixOperation::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternPrefixOperation::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternPrefixOperation::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}
	
	PatternBackRef::PatternBackRef(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void PatternBackRef::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternBackRef::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	PatternBinaryOperation::PatternBinaryOperation(Token&& op, PatternExpression& left, PatternExpression& right)
		: _op(std::move(op)), _left(left), _right(right)
	{
	}

	void PatternBinaryOperation::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternBinaryOperation::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternBinaryOperation::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_left.traverse(visitor);
		_right.traverse(visitor);
	}
	
	PatternLazy::PatternLazy(PatternExpression& expression)
		: _expression(expression)
	{
	}

	void PatternLazy::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternLazy::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternLazy::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}
	
	PatternCapture::PatternCapture(Token&& identifier, PatternExpression& expression)
		: _identifier(std::move(identifier)), _expression(expression)
	{
	}

	void PatternCapture::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternCapture::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PatternCapture::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}

	AppendStatement::AppendStatement(PatternExpression& expression)
		: _expression(expression)
	{
	}

	void AppendStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void AppendStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void AppendStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_expression.traverse(visitor);
	}

	FindStatement::FindStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void FindStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void FindStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	FilterStatement::FilterStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void FilterStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void FilterStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	ReplaceStatement::ReplaceStatement(Expression& match, Expression& string)
		: _match(match), _string(string)
	{
	}

	void ReplaceStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ReplaceStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ReplaceStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_match.traverse(visitor);
		_string.traverse(visitor);
	}
	
	ApplyStatement::ApplyStatement(Token&& identifier)
		: _identifier(std::move(identifier))
	{
	}

	void ApplyStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ApplyStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	ScopeStatement::ScopeStatement(Token&& specifier, Expression& range)
		: _specifier(std::move(specifier)), _range(range)
	{
	}

	void ScopeStatement::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ScopeStatement::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void ScopeStatement::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_range.traverse(visitor);
	}
	
	PagePush::PagePush(Expression& page)
		: _page(page)
	{
	}

	void PagePush::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PagePush::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PagePush::traverse(ASTVisitor& visitor) const
	{
		ASTNode::traverse(visitor);
		_page.traverse(visitor);
	}

	void PagePop::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PagePop::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PageClearStack::pre_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}

	void PageClearStack::post_analyse(RuntimeEnvironment& env, std::vector<LxError>& errors) const
	{
		// TODO
	}
}
