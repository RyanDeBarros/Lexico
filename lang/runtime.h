#pragma once

#include "token.h"
#include "errors.h"

#include <unordered_map>
#include <optional>

namespace lx
{
	enum class DataType
	{
		Int = TokenType::IntType,
		Float = TokenType::FloatType,
		Bool = TokenType::BoolType,
		String = TokenType::StringType,
		Void = TokenType::VoidType,
		Pattern = TokenType::PatternType,
		Match = TokenType::MatchType,
		Matches = TokenType::MatchesType,
		CapId = TokenType::CapIdType,
		Cap = TokenType::CapType,
		IRange = TokenType::IRangeType,
		SRange = TokenType::SRangeType,
		List = TokenType::ListType,
	};

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

	class SymbolTable
	{
		struct TransparentHash
		{
			using is_transparent = void;

			size_t operator()(const std::string_view sv) const
			{
				return std::hash<std::string_view>{}(sv);
			}

			size_t operator()(const std::string& s) const
			{
				return std::hash<std::string>{}(s);
			}
		};

		struct TransparentEqual
		{
			using is_transparent = void;

			bool operator()(std::string_view a, std::string_view b) const
			{
				return a == b;
			}
		};

		std::unordered_map<std::string, VariableSignature, TransparentHash, TransparentEqual> _variable_table;
		std::unordered_map<std::string, FunctionSignature, TransparentHash, TransparentEqual> _function_table;

	public:
		std::optional<unsigned int> identifier_is_registered(const std::string_view identifier) const;

		std::optional<VariableSignature> variable_is_registered(const std::string_view identifier) const;
		bool variable_has_type(const std::string_view identifier, DataType type) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number);

		std::optional<FunctionSignature> function_is_registered(const std::string_view identifier) const;
		bool function_has_return_type(const std::string_view identifier, DataType type) const;
		bool function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types) const;
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
		Isolated,
		Unknown
	};

	class RuntimeEnvironment
	{
		SymbolTable _global_table;
		std::vector<ScopeContext> _scope_stack;
		mutable std::vector<LxError> _errors;
		const std::vector<std::string_view>& _script_lines;

	public:
		RuntimeEnvironment(const std::vector<std::string_view>& script_lines);

		std::vector<LxError>& errors() const;
		const std::vector<std::string_view>& script_lines() const;

		void push_local_scope(bool isolated);
		void pop_local_scope();

		std::optional<unsigned int> identifier_is_registered(const std::string_view identifier, Namespace ns) const;

		std::optional<VariableSignature> variable_is_registered(const std::string_view identifier, Namespace ns) const;
		bool variable_has_type(const std::string_view identifier, DataType type, Namespace ns) const;
		void register_variable(const std::string_view identifier, DataType type, unsigned int line_number, Namespace ns);

		std::optional<FunctionSignature> function_is_registered(const std::string_view identifier, Namespace ns) const;
		bool function_has_return_type(const std::string_view identifier, DataType type, Namespace ns) const;
		bool function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types, Namespace ns) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number, Namespace ns);
	};
}
