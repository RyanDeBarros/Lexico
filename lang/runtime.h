#pragma once

#include "operations.h"
#include "errors.h"

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

	struct FunctionCallSignature
	{
		std::string identifier;
		std::vector<DataType> arg_types;
	};

	struct FunctionCallHash
	{
		size_t operator()(const FunctionCallSignature& fc) const;
	};

	struct FunctionCallEqual
	{
		bool operator()(const FunctionCallSignature& a, const FunctionCallSignature& b) const;
	};

	using FunctionCallSet = std::unordered_set<FunctionCallSignature, FunctionCallHash, FunctionCallEqual>;

	class SymbolTable
	{
		struct TransparentHash
		{
			using is_transparent = void;

			size_t operator()(const std::string_view sv) const;
			size_t operator()(const std::string& s) const;
		};

		struct TransparentEqual
		{
			using is_transparent = void;

			bool operator()(std::string_view a, std::string_view b) const;
		};

		std::unordered_map<std::string, VariableSignature, TransparentHash, TransparentEqual> _variable_table;
		std::unordered_map<FunctionCallSignature, FunctionSignature, FunctionCallHash, FunctionCallEqual> _function_table;
		std::unordered_map<std::string, FunctionCallSet, TransparentHash, TransparentEqual> _function_lut;

	public:
		std::optional<VariableSignature> registered_variable(const std::string_view identifier) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number);

		std::optional<FunctionSignature> registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number);
	};

	struct ScopeContext
	{
		SymbolTable table;
		bool isolated;
	};

	enum class Namespace
	{
		Global,
		Local,
		Unknown
	};

	class RuntimeEnvironment
	{
		SymbolTable _global_table;
		std::vector<ScopeContext> _scope_stack;
		mutable std::vector<LxError> _errors;
		const std::vector<std::string_view>* _script_lines;

	public:
		std::vector<LxError>& errors() const;
		void add_semantic_error(const Token& token, const std::string_view cause) const;

		const std::vector<std::string_view>& script_lines() const;
		void set_script_lines(const std::vector<std::string_view>& script_lines);

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
