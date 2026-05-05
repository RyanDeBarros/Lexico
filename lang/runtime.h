#pragma once

#include "util.h"
#include "namespace.h"
#include "types/heap.h"

#include <unordered_map>

namespace lx
{
	class RuntimeSymbolTable
	{
		StringMap<DataPointHandle> _variable_table;

	public:
		RuntimeSymbolTable() = default;
		RuntimeSymbolTable(const RuntimeSymbolTable&) = delete;
		RuntimeSymbolTable(RuntimeSymbolTable&&) noexcept = default;

		void register_variable(const std::string_view identifier, DataPointHandle&& handle);
		std::optional<DataPointHandle> registered_variable(const std::string_view identifier) const;
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
		std::stringstream _output;
		std::stringstream _log;

		mutable DataHeap _heap;
		RuntimeSymbolTable _global_table;
		std::vector<RuntimeScopeContext> _scope_stack;

		DataPointHandle _global_matches;
		Scope _search_scope;

		StringMap<unsigned int> _capture_ids;

	public:
		Runtime(const std::string_view input);
		
		const std::stringstream& output() const;
		std::stringstream& output();
		const std::stringstream& log() const;
		std::stringstream& log();

		void push_local_scope(bool isolated);
		void pop_local_scope();

		unsigned int scope_depth() const;

		void register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns);
		DataPointHandle registered_variable(const std::string_view identifier, Namespace ns) const;
		DataPointHandle temporary_variable(DataPoint&& dp) const;

		const Matches& global_matches() const;
		Matches& global_matches();
		DataPointHandle global_matches_handle() const;

		const Scope& search_scope() const;
		Scope& search_scope();

		CapId capture_id(const std::string_view id);
	};
}
