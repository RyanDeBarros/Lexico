#include "ftable.h"

#include "ast.h"

namespace lx
{
	size_t FunctionCallHash::operator()(const FunctionCallSignature& fc) const
	{
		size_t h = std::hash<std::string>{}(fc.identifier);
		for (size_t i = 0; i < fc.arg_types.size(); ++i)
			h ^= std::hash<DataType>{}(fc.arg_types[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
		return h;
	}

	bool FunctionCallEqual::operator()(const FunctionCallSignature& a, const FunctionCallSignature& b) const
	{
		return a.identifier == b.identifier && a.arg_types == b.arg_types;
	}

	std::vector<FunctionSignature> SemanticFunctionTable::registered_functions(const std::string_view identifier, const std::vector<DataType>& arg_types) const
	{
		auto it = _map.find(FunctionCallSignature{ .identifier = std::string(identifier), .arg_types = arg_types });
		if (it != _map.end())
			return { it->second };
		else
		{
			auto it = _lut.find(identifier);
			if (it != _lut.end())
			{
				std::vector<FunctionSignature> matching_signatures;
				for (const auto& call : it->second)
				{
					if (call.arg_types.size() != arg_types.size())
						continue;

					bool match = true;
					for (size_t i = 0; i < arg_types.size(); ++i)
					{
						if (arg_types[i] != DataType::_Unresolved && arg_types[i] != call.arg_types[i])
						{
							match = false;
							break;
						}
					}
					
					if (match)
						matching_signatures.push_back(_map.find(call)->second);
				}
				return matching_signatures;
			}
			else
				return {};
		}
	}

	std::optional<FunctionSignature> SemanticFunctionTable::known_registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const
	{
		auto it = _map.find(FunctionCallSignature{ .identifier = std::string(identifier), .arg_types = arg_types });
		if (it != _map.end())
			return { it->second };
		else
			return std::nullopt;
	}

	FunctionCallSet SemanticFunctionTable::registered_function_calls(const std::string_view identifier) const
	{
		auto it = _lut.find(identifier);
		if (it != _lut.end())
			return it->second;
		else
			return {};
	}

	void SemanticFunctionTable::register_function(FunctionDefinition& decl_node, const std::string_view identifier,
		DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number)
	{
		if (!_lut.count(identifier))
			_lut[std::string(identifier)] = {};

		auto& registered_args = _lut.find(identifier)->second;
		FunctionCallSignature fc{ .identifier = std::string(identifier), .arg_types = arg_types };
		if (registered_args.count(fc))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		registered_args.insert(fc);
		_map.try_emplace(std::move(fc), &decl_node, line_number, return_type, std::move(arg_types));
	}

	RuntimeFunctionTable::RuntimeFunctionTable(const SemanticFunctionTable& semantic_table)
	{
		// TODO load map
	}

	const FunctionDefinition* RuntimeFunctionTable::registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const
	{
		auto it = _map.find(FunctionCallSignature{ .identifier = std::string(identifier), .arg_types = arg_types });
		if (it != _map.end())
			return it->second.decl_node;
		else
			return nullptr;
	}
}
