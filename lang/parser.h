#pragma once

#include "lexer.h"
#include "ast.h"
#include "errors.h"

namespace lx
{
	class Parser
	{
		AbstractSyntaxTree _tree;
		std::vector<LxError> _errors;

	public:
		Parser(Token&& start_token);

		void parse(Lexer& lexer);

		const AbstractSyntaxTree& tree() const;
		AbstractSyntaxTree& tree();
		const std::vector<LxError>& errors() const;
	};
}
