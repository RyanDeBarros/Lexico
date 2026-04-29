#include "semantics.h"

namespace lx
{
	class AnalysisVisitor : public ASTVisitor
	{
		RuntimeEnvironment& _env;

	public:
		AnalysisVisitor(RuntimeEnvironment& env)
			: _env(env)
		{
		}

		void pre_visit(const ASTNode& node) override
		{
			if (!node.validated())
				node.pre_analyse(_env);
		}

		void post_visit(const ASTNode& node) override
		{
			if (node.validated())
				node.post_analyse(_env);
		}
	};

	void SemanticAnalyser::analyse(Parser& parser)
	{
		AnalysisVisitor visitor(_env);
		parser.tree().root().accept(visitor);
	}

	const std::vector<LxError>& SemanticAnalyser::errors() const
	{
		return _env.errors();
	}
}
