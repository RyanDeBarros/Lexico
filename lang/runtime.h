#pragma once

#include <unordered_map>

#include "util.h"
#include "namespace.h"
#include "types/datapoint.h"

namespace lx
{
	class Variable
	{
		std::unique_ptr<DataPoint> _dp;

	public:
		Variable(const DataPoint& dp);
		Variable(DataPoint&& dp);

		Variable(const Variable&) = delete;
		Variable(Variable&&) noexcept = default;

		const DataPoint& ref() const;
		DataPoint& ref();
	};

	class RuntimeSymbolTable
	{
		StringMap<Variable> _variable_table;

	public:
		RuntimeSymbolTable() = default;
		RuntimeSymbolTable(const RuntimeSymbolTable&) = delete;
		RuntimeSymbolTable(RuntimeSymbolTable&&) noexcept = default;

		void register_variable(const std::string_view identifier, DataPoint&& dp);
		const DataPoint* registered_variable(const std::string_view identifier) const;
		DataPoint* registered_variable(const std::string_view identifier);
	};

	struct RuntimeScopeContext
	{
		RuntimeSymbolTable table;
		bool isolated;

		RuntimeScopeContext(bool isolated);
		RuntimeScopeContext(const RuntimeScopeContext&) = delete;
		RuntimeScopeContext(RuntimeScopeContext&&) noexcept = default;
	};

	// TODO even in SemanticContext, functions should not be declared within one another. All functions should go into a global function table, and Runtime should load that table directly from the built SemanticContext. Stored functions should also store a const FunctionDefinition* node. Should do a full pass first to build the funciton table before normal analysis, so that functions can be declared in any order. So perhaps they should be separate from SemanticContext, and only have a reference to the FunctionTable in SemanticContext/Runtime.

	class Runtime
	{
		const std::string_view _input;
		std::stringstream& _output;
		std::stringstream& _log;

		RuntimeSymbolTable _global_table;
		std::vector<RuntimeScopeContext> _scope_stack;

		Matches _global_matches;
		Scope _search_scope;

		StringMap<unsigned int> _capture_ids;

	public:
		Runtime(const std::string_view input, std::stringstream& output, std::stringstream& log);

		void push_local_scope(bool isolated);
		void pop_local_scope();

		unsigned int scope_depth() const;

		void register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns);
		const DataPoint& registered_variable(const std::string_view identifier, Namespace ns) const;
		DataPoint& registered_variable(const std::string_view identifier, Namespace ns);

		const Matches& global_matches() const;
		Matches& global_matches();

		const Scope& search_scope() const;
		Scope& search_scope();

		CapId capture_id(const std::string_view id);
	};
}
