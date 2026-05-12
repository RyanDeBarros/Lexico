#pragma once

#include "util.h"
#include "namespace.h"
#include "types/heap.h"
#include "ftable.h"
#include "page.h"

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

	class Runtime
	{
		const std::string_view _input;
		std::stringstream _output;
		std::stringstream _log;

		VirtualHeap _heap;
		RuntimeSymbolTable _global_variable_table;
		std::vector<RuntimeScopeContext> _scope_stack;
		RuntimeFunctionTable _function_table;

		StringMap<Variable> _declared_patterns;
		std::optional<Variable> _focused_pattern;

		Variable _global_matches;
		Scope _search_scope;

		std::unique_ptr<Page> _root_page;
		std::stack<Page> _page_stack;

		StringMap<unsigned int> _capture_ids;

	public:
		Runtime(const std::string_view input, SemanticFunctionTable&& ftable);
		Runtime(const Runtime&) = delete;
		
		const std::stringstream& output() const;
		std::stringstream& output();
		const std::stringstream& log() const;
		std::stringstream& log();

	private:
		void push_local_scope(bool isolated);
		void pop_local_scope();

	public:
		class LocalScope
		{
			Runtime& _runtime;
			bool _alive = false;

		public:
			LocalScope(Runtime& runtime, bool isolated);
			LocalScope(const LocalScope&) = delete;
			LocalScope(LocalScope&&) noexcept;
			~LocalScope();
			LocalScope& operator=(LocalScope&&) = delete;
		};

		void register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns);
		Variable registered_variable(const std::string_view identifier, Namespace ns, const ScriptSegment& segment) const;
		Variable unbound_variable(DataPoint&& dp);

		const FunctionDefinition& registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types, const ScriptSegment& segment) const;

		void declare_pattern(std::string_view identifier);
		void delete_pattern(std::string_view identifier);
		Variable focused_pattern(const ScriptSegment& segment) const;

		void find(const ScriptSegment& segment);
		void add_highlight(const Color& color, const std::optional<Variable>& format);
		void remove_highlight(const Color& color, const std::optional<Variable>& format);

		void push_page(Variable page_desc, const ScriptSegment& segment);
		void pop_page(const ScriptSegment& segment);
		void clear_page_stack();
		const Page& focused_page() const;

		const Matches& global_matches() const;
		Matches& global_matches();
		Variable global_matches_var() const;

		const Scope& search_scope() const;
		Scope& search_scope();

		CapId capture_id(const std::string_view id);
	};
}
