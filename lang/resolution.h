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

	class SemanticSymbolTable
	{
		StringMap<VariableSignature> _variable_table;
		std::unordered_map<FunctionCallSignature, FunctionSignature, FunctionCallHash, FunctionCallEqual> _function_table;
		StringMap<FunctionCallSet> _function_lut;

	public:
		std::optional<VariableSignature> registered_variable(const std::string_view identifier) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number);

		std::optional<FunctionSignature> registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number);
	};

	struct SemanticScopeContext
	{
		SemanticSymbolTable table;
		bool isolated;
	};

	class SemanticContext
	{
		SemanticSymbolTable _global_table;
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

		void push_local_scope(bool isolated);
		void pop_local_scope();

		unsigned int scope_depth() const;

		std::optional<unsigned int> identifier_first_decl_line_number(const std::string_view identifier, Namespace ns) const;

		std::optional<VariableSignature> registered_variable(const std::string_view identifier, Namespace ns) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number, Namespace ns);

		std::optional<FunctionSignature> registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types, Namespace ns) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier, Namespace ns) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number, Namespace ns);
	};
}
