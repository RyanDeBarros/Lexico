#include "semantics.h"

namespace lx
{
	class AnalysisVisitor : public ASTVisitor
	{
		ResolutionContext& _ctx;

	public:
		AnalysisVisitor(ResolutionContext& ctx)
			: _ctx(ctx)
		{
		}

		void pre_visit(ASTNode& node) override
		{
			if (!node.validated())
				node.pre_analyse(_ctx);
		}

		void post_visit(ASTNode& node) override
		{
			if (node.validated())
			{
				try
				{
					node.post_analyse(_ctx);
				}
				catch (const LxError& e)
				{
					// TODO better error propogation/ignoring/duplication handling
					if (e.type() != ErrorType::Internal)
						_ctx.errors().push_back(e);
					// TODO v0.2 optional debug log for else branch
				}
			}
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
