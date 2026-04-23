#pragma once

#include <memory>
#include <vector>
#include <stack>

namespace lx
{
	class ASTNode
	{
		ASTNode* _parent = nullptr;
		std::vector<ASTNode*> _children;

	public:
		virtual ~ASTNode() = default;

		void append(ASTNode& child);
		size_t children_count() const;
		const ASTNode& child(size_t i) const;
		ASTNode& child(size_t i);
		bool has_parent() const;
		const ASTNode& parent() const;
		ASTNode& parent();
	};

	class AbstractSyntaxTree
	{
		std::vector<std::unique_ptr<ASTNode>> _nodes;

		class ConstIterator
		{
			const AbstractSyntaxTree& _tree;
			std::stack<size_t> _index_in_parent;
			ASTNode* _current_node;

		public:
			ConstIterator(const AbstractSyntaxTree& tree);
			
			bool eof() const;
			void advance();
			const ASTNode& operator*() const;
			const ASTNode* operator->() const;
		};

	public:
		ASTNode& add(std::unique_ptr<ASTNode>&& node);
		ConstIterator iterator() const;
	};
}
