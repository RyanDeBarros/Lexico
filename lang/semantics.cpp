#include "semantics.h"

namespace lx
{
	void SemanticAnalyser::analyse(Parser& parser)
	{
		SemanticContext dry_ctx;
		parser.tree().root().validate(dry_ctx);
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
