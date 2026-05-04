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
		Runtime env(input, _output_stream, _log_stream);
		_ast.execute(env);
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
