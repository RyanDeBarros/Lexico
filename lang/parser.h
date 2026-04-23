#pragma once

#include "token.h"
#include "ast.h"

namespace lx
{
	class Parser
	{
		AbstractSyntaxTree _tree;

	public:
		void parse(TokenStream& stream);

		const AbstractSyntaxTree& tree() const;
		AbstractSyntaxTree& tree();
	};
}
