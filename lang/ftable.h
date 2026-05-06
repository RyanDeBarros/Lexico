#pragma once

#include "types/datatype.h"
#include "util.h"

#include <optional>

namespace lx
{
	class FunctionDefinition;

	struct FunctionSignature
	{
		FunctionDefinition* decl_node = nullptr;
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

	class SemanticFunctionTable
	{
		// TODO don't use map so as to be able to match unresolved arguments -> only for SemanticContext. For execution, unresolved types will be resolved, so can use a map. Therefore, distinguish between SemanticFunctionTable and RuntimeFunctionTable.
		std::unordered_map<FunctionCallSignature, FunctionSignature, FunctionCallHash, FunctionCallEqual> _map;
		StringMap<FunctionCallSet> _lut;

	public:
		std::vector<FunctionSignature> registered_functions(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		std::optional<FunctionSignature> known_registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
		FunctionCallSet registered_function_calls(const std::string_view identifier) const;
		void register_function(FunctionDefinition& decl_node, const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number);
	};

	class RuntimeFunctionTable
	{
		std::unordered_map<FunctionCallSignature, FunctionSignature, FunctionCallHash, FunctionCallEqual> _map;

	public:
		RuntimeFunctionTable(const SemanticFunctionTable& semantic_table);

		const FunctionDefinition* registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const;
	};
}
