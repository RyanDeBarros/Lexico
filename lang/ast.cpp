#include "ast.h"

#include <sstream>
#include <stdexcept>

namespace lx
{
	void ASTNode::append(ASTNode& child)
	{
		if (child._parent)
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": child already has a parent";
			throw std::runtime_error(ss.str());
		}

		child._parent = this;
		_children.push_back(&child);
	}

	size_t ASTNode::children_count() const
	{
		return _children.size();
	}
	
	const ASTNode& ASTNode::child(size_t i) const
	{
		return *_children[i];
	}

	ASTNode& ASTNode::child(size_t i)
	{
		return *_children[i];
	}

	bool ASTNode::has_parent() const
	{
		return _parent != nullptr;
	}

	const ASTNode& ASTNode::parent() const
	{
		if (!has_parent())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": no parent";
			throw std::runtime_error(ss.str());
		}

		return *_parent;
	}

	ASTNode& ASTNode::parent()
	{
		if (!has_parent())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": no parent";
			throw std::runtime_error(ss.str());
		}

		return *_parent;
	}

	AbstractSyntaxTree::ConstIterator::ConstIterator(const AbstractSyntaxTree& tree)
		: _tree(tree), _current_node(!tree._nodes.empty() ? _current_node = tree._nodes[0].get() : nullptr)
	{
	}

	bool AbstractSyntaxTree::ConstIterator::eof() const
	{
		return _current_node != nullptr;
	}
	
	void AbstractSyntaxTree::ConstIterator::advance()
	{
		if (eof())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": at eof()";
			throw std::runtime_error(ss.str());
		}

		if (_current_node->children_count() > 0)
		{
			_index_in_parent.push(0);
			_current_node = &_current_node->child(0);
		}
		else
		{
			while (_current_node->has_parent())
			{
				_current_node = &_current_node->parent();
				size_t sibling_index = _index_in_parent.top() + 1;
				_index_in_parent.pop();
				if (sibling_index < _current_node->children_count())
				{
					_index_in_parent.push(sibling_index);
					_current_node = &_current_node->child(sibling_index);
					break;
				}
			}

			if (!_current_node->has_parent())
				_current_node = nullptr;
		}
	}
	
	const ASTNode& AbstractSyntaxTree::ConstIterator::operator*() const
	{
		if (eof())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": at eof()";
			throw std::runtime_error(ss.str());
		}

		return *_current_node;
	}
	
	const ASTNode* AbstractSyntaxTree::ConstIterator::operator->() const
	{
		if (eof())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": at eof()";
			throw std::runtime_error(ss.str());
		}

		return _current_node;
	}

	ASTNode& AbstractSyntaxTree::add(std::unique_ptr<ASTNode>&& node)
	{
		ASTNode* ref = node.get();
		_nodes.push_back(std::move(node));
		return *ref;
	}

	AbstractSyntaxTree::ConstIterator AbstractSyntaxTree::iterator() const
	{
		return ConstIterator(*this);
	}
}
