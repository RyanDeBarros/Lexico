#pragma once

#include "parser.h"
#include "runtime.h"

namespace lx
{
	class SemanticAnalyser
	{
		std::vector<LxError> _errors;

	public:
		void analyse(Parser& parser);

		const std::vector<LxError>& errors() const;
	};
}
