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

	class Runtime
	{
		const std::string_view _input;
		std::stringstream& _output;
		std::stringstream& _log;

		RuntimeSymbolTable _global_table;
		std::vector<RuntimeScopeContext> _scope_stack;

		DataPoint _global_matches;

		StringMap<unsigned int> _capture_ids;

	public:
		Runtime(const std::string_view input, std::stringstream& output, std::stringstream& log);

		void push_local_scope(bool isolated);
		void pop_local_scope();

		unsigned int scope_depth() const;

		void register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns);
		const DataPoint& registered_variable(const std::string_view identifier, Namespace ns) const;
		DataPoint& registered_variable(const std::string_view identifier, Namespace ns);

		const DataPoint& global_matches() const;
		DataPoint& global_matches();

		CapId capture_id(const std::string_view id);
	};
}
