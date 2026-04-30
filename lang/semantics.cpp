#include "semantics.h"

namespace lx
{
	class AnalysisVisitor : public ASTVisitor
	{
		ResolutionContext& _env;

	public:
		AnalysisVisitor(ResolutionContext& env)
			: _env(env)
		{
		}

		void pre_visit(ASTNode& node) override
		{
			if (!node.validated())
				node.pre_analyse(_env);
		}

		void post_visit(ASTNode& node) override
		{
			if (node.validated())
				node.post_analyse(_env);
		}
	};

	void SemanticAnalyser::analyse(Parser& parser)
	{
		ResolutionContext dry_ctx;
		AnalysisVisitor visitor(dry_ctx);
		parser.tree().root().accept(visitor);
		_errors = std::move(dry_ctx.errors());
		_warnings = std::move(dry_ctx.warnings());
	}

	const std::vector<LxError>& SemanticAnalyser::errors() const
	{
		return _errors;
	}

	const std::vector<LxWarning>& SemanticAnalyser::warnings() const
	{
		return _warnings;
	}
}
