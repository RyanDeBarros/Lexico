#pragma once

#include "token.h"
#include "ast.h"
#include "errors.h"

namespace lx
{
	class Parser
	{
		AbstractSyntaxTree _tree;
		std::vector<SyntaxError> _errors;

	public:
		void parse(TokenStream& stream, const std::vector<std::string_view>& script_lines);

		const AbstractSyntaxTree& tree() const;
		const std::vector<SyntaxError>& errors() const;
	};
}
