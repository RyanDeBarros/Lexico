#include "semantics.h"

namespace lx
{
	class AnalysisVisitor : public ASTVisitor
	{
		RuntimeEnvironment& _env;
		std::vector<LxError>& _errors;

	public:
		AnalysisVisitor(RuntimeEnvironment& env, std::vector<LxError>& errors)
			: _env(env), _errors(errors)
		{
		}

		void visit(const ASTNode& node) override
		{
			node.analyse(_env, _errors);
		}
	};

	void SemanticAnalyser::analyse(Parser& parser)
	{
		AnalysisVisitor visitor(_env, _errors);
		parser.tree().root().accept(visitor);
	}

	const RuntimeEnvironment& SemanticAnalyser::env() const
	{
		return _env;
	}

	const std::vector<LxError>& SemanticAnalyser::errors() const
	{
		return _errors;
	}
}
