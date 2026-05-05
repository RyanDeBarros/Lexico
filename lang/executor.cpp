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
		Runtime env(input);
		_ast.execute(env);
		_output_stream = std::move(env.output());
		_log_stream = std::move(env.log());
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
