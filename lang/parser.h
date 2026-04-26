#pragma once

#include "lexer.h"
#include "ast.h"
#include "errors.h"

namespace lx
{
	class Parser
	{
		AbstractSyntaxTree _tree;
		std::vector<SyntaxError> _errors;

	public:
		void parse(Lexer& lexer);

		const AbstractSyntaxTree& tree() const;
		const std::vector<SyntaxError>& errors() const;
	};
}
