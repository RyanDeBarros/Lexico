#pragma once

#include "parser.h"
#include "runtime.h"

namespace lx
{
	class SemanticAnalyser
	{
		RuntimeEnvironment _env;
		std::vector<LxError> _errors;

	public:
		void analyse(Parser& parser);

		const RuntimeEnvironment& env() const;
		const std::vector<LxError>& errors() const;
	};
}
