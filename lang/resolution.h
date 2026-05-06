#pragma once

#include "operations.h"
#include "errors.h"
#include "util.h"
#include "namespace.h"

#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace lx
{
	struct VariableSignature
	{
		unsigned int decl_line_number = 0;
		DataType type = DataType::Void;
	};

	struct FunctionSignature
	{
		unsigned int decl_line_number = 0;
		DataType return_type = DataType::Void;
		std::vector<DataType> arg_types;
	};

	class SemanticVariableTable
	{
		StringMap<VariableSignature> _map;

	public:
		std::optional<VariableSignature> registered_variable(const std::string_view identifier) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number);
	};

	class FunctionTable
	{
		// TODO don't use map so as to be able to match unresolved arguments
		std::unordered_map<FunctionCallSignature, FunctionSignature, FunctionCallHash, FunctionCallEqual> _map;
		StringMap<FunctionCallSet> _lut;

	public:
		std::optional<FunctionSignature> registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number);
	};

	struct SemanticScopeContext
	{
		SemanticVariableTable table;
		bool isolated;
	};

	class SemanticContext
	{
		SemanticVariableTable _global_variable_table;
		FunctionTable _function_table;
		std::vector<SemanticScopeContext> _scope_stack;
		mutable std::vector<LxError> _errors;
		mutable std::vector<LxWarning> _warnings;

	public:
		std::vector<LxError>& errors() const;
		void add_semantic_error(const Token& token, const std::string_view cause) const;
		void add_semantic_error(const ScriptSegment& segment, const std::string_view cause) const;

		std::vector<LxWarning>& warnings() const;
		void add_semantic_warning(const Token& token, const std::string_view cause) const;
		void add_semantic_warning(const ScriptSegment& segment, const std::string_view cause) const;

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

		std::optional<unsigned int> identifier_first_decl_line_number(const std::string_view identifier, Namespace ns) const;

		std::optional<VariableSignature> registered_variable(const std::string_view identifier, Namespace ns) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number, Namespace ns);

		std::optional<FunctionSignature> registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number);
	};
}
