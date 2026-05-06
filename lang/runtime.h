#pragma once

#include "util.h"
#include "namespace.h"
#include "types/heap.h"

#include <unordered_map>

namespace lx
{
	class RuntimeSymbolTable
	{
		StringMap<Variable> _variable_table;

	public:
		RuntimeSymbolTable() = default;
		RuntimeSymbolTable(const RuntimeSymbolTable&) = delete;
		RuntimeSymbolTable(RuntimeSymbolTable&&) noexcept = default;

		void register_variable(const std::string_view identifier, Variable&& handle);
		std::optional<Variable> registered_variable(const std::string_view identifier) const;
	};

	struct RuntimeScopeContext
	{
		RuntimeSymbolTable table;
		bool isolated;

		RuntimeScopeContext(bool isolated);
		RuntimeScopeContext(const RuntimeScopeContext&) = delete;
		RuntimeScopeContext(RuntimeScopeContext&&) noexcept = default;
	};

	struct Page
	{
		std::string content;
	};

	// TODO even in SemanticContext, functions should not be declared within one another. All functions should go into a global function table, and Runtime should load that table directly from the built SemanticContext. Stored functions should also store a const FunctionDefinition* node. Should do a full pass first to build the funciton table before normal analysis, so that functions can be declared in any order. So perhaps they should be separate from SemanticContext, and only have a reference to the FunctionTable in SemanticContext/Runtime.

	class Runtime
	{
		const std::string_view _input;
		std::stringstream _output;
		std::stringstream _log;

		DataHeap _heap;
		RuntimeSymbolTable _global_table;
		std::vector<RuntimeScopeContext> _scope_stack;

		StringMap<Variable> _declared_patterns;
		std::optional<Variable> _focused_pattern;

		Variable _global_matches;
		Scope _search_scope;

		Page _root_page;
		std::stack<Page> _page_stack;

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
		Variable registered_variable(const std::string_view identifier, Namespace ns, const ScriptSegment& segment) const;
		Variable temporary_variable(DataPoint&& dp);
		Variable unnamed_variable(Variable& owner, DataPoint&& dp);

		void declare_pattern(std::string_view identifier);
		void delete_pattern(std::string_view identifier);
		Variable focused_pattern(const ScriptSegment& segment) const;

		void find();
		void add_highlight(const Color& color, const std::optional<Variable>& format);
		void remove_highlight(const Color& color, const std::optional<Variable>& format);

		void push_page(const Variable& page_desc);
		void pop_page(const ScriptSegment& segment);
		void clear_page_stack();
		const Page& focused_page() const;

		const Matches& global_matches() const;
		Matches& global_matches();
		Variable global_matches_handle() const;

		const Scope& search_scope() const;
		Scope& search_scope();

		CapId capture_id(const std::string_view id);
	};
}
