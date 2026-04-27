#pragma once

#include "token.h"

#include <unordered_map>

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
		DataType type = DataType::Void;
	};

	struct FunctionSignature
	{
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
		bool variable_is_registered(const std::string_view identifier) const;
		bool variable_has_type(const std::string_view identifier, DataType type) const;
		void register_variable(const std::string_view identifier, DataType type);

		bool function_is_registered(const std::string_view identifier) const;
		bool function_has_return_type(const std::string_view identifier, DataType type) const;
		bool function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types);
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

	public:
		void push_local_scope(bool isolated);
		void pop_local_scope();

		bool variable_is_registered(const std::string_view identifier, Namespace ns) const;
		bool variable_has_type(const std::string_view identifier, DataType type, Namespace ns) const;
		void register_variable(const std::string_view identifier, DataType type, Namespace ns);

		bool function_is_registered(const std::string_view identifier, Namespace ns) const;
		bool function_has_return_type(const std::string_view identifier, DataType type, Namespace ns) const;
		bool function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types, Namespace ns) const;
		void register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, Namespace ns);
	};
}
