#pragma once

#include "semantics.h"

#include <sstream>

namespace lx
{
	class Executor
	{
		ASTRoot& _ast;
		std::stringstream _output_stream;
		std::stringstream _log_stream;

	public:
		Executor(ASTRoot& ast);
		void execute(SemanticAnalyser& analyser, const std::string_view input);

		const std::stringstream& output() const;
		const std::stringstream& log() const;
	};
}
