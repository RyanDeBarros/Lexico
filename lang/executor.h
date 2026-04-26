#pragma once

#include "semantics.h"

#include <sstream>

namespace lx
{
	class Executor
	{
		std::stringstream _output_stream;
		std::stringstream _log_stream;

	public:
		void execute(SemanticAnalyser& analyser, const std::string_view input);

		const std::stringstream& output() const;
		const std::stringstream& log() const;
	};
}
