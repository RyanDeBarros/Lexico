#include "executor.h"

#include "runtime.h"

namespace lx
{
	Executor::Executor(ASTRoot& ast)
		: _ast(ast)
	{
	}

	void Executor::execute(SemanticAnalyser& analyser, const std::string_view input)
	{
		Runtime runtime(input, std::move(analyser.ftable()));
		_ast.execute(runtime); // TODO v0.2 some kind of recursion/infinite-loop/time limit for safety. This can be a configurable setting passed to execute()
		_output_stream = std::move(runtime.output());
		_log_stream = std::move(runtime.log());
		_highlights = std::move(runtime.highlights());
	}

	const std::stringstream& Executor::output() const
	{
		return _output_stream;
	}

	const std::stringstream& Executor::log() const
	{
		return _log_stream;
	}

	HighlightMap& Executor::highlights()
	{
		return _highlights;
	}
}
