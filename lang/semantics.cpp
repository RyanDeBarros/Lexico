#include "semantics.h"

namespace lx
{
	void SemanticAnalyser::analyse(Parser& parser)
	{
		SemanticContext dry_ctx;
		parser.tree().root().analyse_tree(dry_ctx);
		_errors = std::move(dry_ctx.errors());
		_warnings = std::move(dry_ctx.warnings());
		_ftable = std::move(dry_ctx.ftable());
	}

	const std::vector<LxError>& SemanticAnalyser::errors() const
	{
		return _errors;
	}

	const std::vector<LxWarning>& SemanticAnalyser::warnings() const
	{
		return _warnings;
	}

	SemanticFunctionTable SemanticAnalyser::ftable()
	{
		return _ftable;
	}
}
