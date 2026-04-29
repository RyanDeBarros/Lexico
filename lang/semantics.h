#pragma once

#include "parser.h"
#include "runtime.h"

namespace lx
{
	class SemanticAnalyser
	{
		RuntimeEnvironment _env;

	public:
		void analyse(Parser& parser);

		const std::vector<LxError>& errors() const;
	};
}
