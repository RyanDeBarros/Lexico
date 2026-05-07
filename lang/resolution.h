#pragma once

#include "errors.h"
#include "util.h"
#include "namespace.h"
#include "ftable.h"

namespace lx
{
	class FunctionCallExpression;

	struct VariableSignature
	{
		unsigned int decl_line_number = 0;
		DataType type = DataType::Void();
	};

	class SemanticVariableTable
	{
		StringMap<VariableSignature> _map;

	public:
		std::optional<VariableSignature> registered_variable(const std::string_view identifier) const;
		void register_variable(const std::string_view identifier, DataType&& type, unsigned int line_number);
	};

	struct SemanticScopeContext
	{
		SemanticVariableTable table;
		bool isolated;
	};

	class SemanticContext;

	class VarConsistencyTest
	{
		std::unordered_set<const FunctionDefinition*> _seen_functions;
		std::vector<const FunctionCallExpression*> _call_stack;
		StringSet _declared_vars;
		std::vector<StringSet> _declared_locals;

	public:
		bool seen(const FunctionDefinition& fn) const;
		void see(const FunctionDefinition& fn, const FunctionCallExpression& call_site);
		void clear_stack();
		void exit_scope();

		void declare_global(const std::string_view identifier);
		void declare_local(const std::string_view identifier);
		void test(SemanticContext& ctx, const Token& var) const;
	};

	class SemanticContext
	{
		SemanticVariableTable _global_variable_table;
		SemanticFunctionTable _function_table;
		std::vector<SemanticScopeContext> _scope_stack;
		std::vector<LxError> _errors;
		std::vector<LxWarning> _warnings;
		VarConsistencyTest _var_consistency_test;

	public:
		std::vector<LxError>& errors();
		void add_semantic_error(const Token& token, const std::string_view cause);
		void add_semantic_error(const ScriptSegment& segment, const std::string_view cause);

		std::vector<LxWarning>& warnings();
		void add_semantic_warning(const Token& token, const std::string_view cause);
		void add_semantic_warning(const ScriptSegment& segment, const std::string_view cause);

	private:
		void push_local_scope(bool isolated);
		void pop_local_scope();

	public:
		class LocalScope
		{
			SemanticContext& _context;
			bool _alive = false;

		public:
			LocalScope(SemanticContext& context, bool isolated);
			LocalScope(const LocalScope&) = delete;
			LocalScope(LocalScope&&) noexcept;
			~LocalScope();
			LocalScope& operator=(LocalScope&&) = delete;
		};

		bool in_local_scope() const;
		VarConsistencyTest& var_consistency_test();

		std::optional<unsigned int> identifier_first_decl_line_number(const std::string_view identifier, Namespace ns);

		std::optional<VariableSignature> registered_variable(const std::string_view identifier, Namespace ns);
		void register_variable(const std::string_view identifier, DataType&& type, unsigned int line_number, Namespace ns);
		void register_pattern(const std::string_view identifier);

		std::optional<FunctionSignature> registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier) const;
		void register_function(FunctionDefinition& decl_node, const std::string_view identifier, DataType&& return_type, std::vector<DataType>&& arg_types, unsigned int line_number);
		SemanticFunctionTable& ftable();
	};
}
