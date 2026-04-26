#include "executor.h"

namespace lx
{
	void Executor::execute(SemanticAnalyser& analyser, const std::string_view input)
	{
		// TODO
		_output_stream << input;
	}

	const std::stringstream& Executor::output() const
	{
		return _output_stream;
	}

	const std::stringstream& Executor::log() const
	{
		return _log_stream;
	}
}
