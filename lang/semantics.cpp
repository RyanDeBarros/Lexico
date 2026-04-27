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
			node.pre_analyse(_env);
		}

		void post_visit(const ASTNode& node) override
		{
			node.post_analyse(_env);
		}
	};

	void SemanticAnalyser::analyse(Parser& parser, const std::vector<std::string_view>& script_lines)
	{
		RuntimeEnvironment dry(script_lines);
		AnalysisVisitor visitor(dry);
		parser.tree().root().accept(visitor);
		_errors.insert(_errors.begin(), dry.errors().begin(), dry.errors().end());
	}

	const std::vector<LxError>& SemanticAnalyser::errors() const
	{
		return _errors;
	}
}
