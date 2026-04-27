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

		void pre_visit(const ASTNode& node) override
		{
			node.pre_analyse(_env, _errors);
		}

		void post_visit(const ASTNode& node) override
		{
			node.post_analyse(_env, _errors);
		}
	};

	void SemanticAnalyser::analyse(Parser& parser)
	{
		RuntimeEnvironment dry;
		AnalysisVisitor visitor(dry, _errors);
		parser.tree().root().accept(visitor);
	}

	const std::vector<LxError>& SemanticAnalyser::errors() const
	{
		return _errors;
	}
}
