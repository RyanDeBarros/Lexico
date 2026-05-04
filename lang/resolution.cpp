#include "resolution.h"

#include "errors.h"

#include <sstream>

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

	std::optional<VariableSignature> SemanticSymbolTable::registered_variable(const std::string_view identifier) const
	{
		auto it = _variable_table.find(identifier);
		if (it != _variable_table.end())
			return it->second;
		else
			return std::nullopt;
	}

	void SemanticSymbolTable::register_variable(const std::string_view identifier, DataType type, unsigned int line_number)
	{
		if (_variable_table.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		_variable_table[std::string(identifier)] = { .decl_line_number = line_number, .type = type };
	}

	std::optional<FunctionSignature> SemanticSymbolTable::registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types) const
	{
		auto it = _function_table.find(FunctionCallSignature{ .identifier = std::string(identifier), .arg_types = arg_types });
		if (it != _function_table.end())
			return it->second;
		else
			return std::nullopt;
	}

	FunctionCallSet SemanticSymbolTable::registered_function_calls(const std::string_view identifier) const
	{
		auto it = _function_lut.find(identifier);
		if (it != _function_lut.end())
			return it->second;
		else
			return {};
	}

	void SemanticSymbolTable::register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number)
	{
		if (!_function_lut.count(identifier))
			_function_lut[std::string(identifier)] = {};

		auto& registered_args = _function_lut.find(identifier)->second;
		FunctionCallSignature fc{ .identifier = std::string(identifier), .arg_types = arg_types };
		if (registered_args.count(fc))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		registered_args.insert(fc);
		_function_table[std::move(fc)] = { .decl_line_number = line_number, .return_type = return_type, .arg_types = std::move(arg_types) };
	}

	std::vector<LxError>& SemanticContext::errors() const
	{
		return _errors;
	}

	void SemanticContext::add_semantic_error(const ScriptSegment& segment, const std::string_view cause) const
	{
		_errors.push_back(LxError::segment_error(segment, ErrorType::Semantic, cause));
	}

	void SemanticContext::add_semantic_error(const Token& token, const std::string_view cause) const
	{
		add_semantic_error(token.segment, cause);
	}

	std::vector<LxWarning>& SemanticContext::warnings() const
	{
		return _warnings;
	}

	void SemanticContext::add_semantic_warning(const ScriptSegment& segment, const std::string_view cause) const
	{
		_warnings.push_back(LxWarning::segment_warning(segment, ErrorType::Semantic, cause));
	}

	void SemanticContext::add_semantic_warning(const Token& token, const std::string_view cause) const
	{
		add_semantic_warning(token.segment, cause);
	}

	void SemanticContext::push_local_scope(bool isolated)
	{
		_scope_stack.push_back({ .isolated = isolated });
	}

	void SemanticContext::pop_local_scope()
	{
		if (_scope_stack.empty())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": scope stack is empty";
			throw LxError(ErrorType::Internal, ss.str());
		}
		_scope_stack.pop_back();
	}

	unsigned int SemanticContext::scope_depth() const
	{
		return _scope_stack.size();
	}

	static std::optional<unsigned int> first_line_number(const SemanticSymbolTable& table, const std::string_view identifier)
	{
		std::optional<unsigned int> first_line_number = std::nullopt;

		if (auto var = table.registered_variable(identifier))
			first_line_number = var->decl_line_number;

		auto calls = table.registered_function_calls(identifier);
		if (!calls.empty())
		{
			if (!first_line_number)
				first_line_number = std::numeric_limits<unsigned int>::max();

			for (const auto& call : calls)
				first_line_number = std::min(*first_line_number, table.registered_function(call.identifier, call.arg_types)->decl_line_number);
		}

		return first_line_number;
	}

	std::optional<unsigned int> SemanticContext::identifier_first_decl_line_number(const std::string_view identifier, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return first_line_number(_global_table, identifier);

			std::optional<unsigned int> line_number = std::nullopt;

			if (ns == Namespace::Unknown || scope_depth() == 0)
				line_number = first_line_number(_global_table, identifier);

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto ln = first_line_number(it->table, identifier))
				{
					if (line_number)
						line_number = std::min(*line_number, *ln);
					else
						line_number = ln;
				}

				if (it->isolated)
					break;
			}

			return line_number;
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return {};
	}

	std::optional<VariableSignature> SemanticContext::registered_variable(const std::string_view identifier, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.registered_variable(identifier);
			
			if (ns == Namespace::Unknown || scope_depth() == 0)
			{
				if (auto sig = _global_table.registered_variable(identifier))
					return sig;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto sig = it->table.registered_variable(identifier))
					return sig;
				else if (it->isolated)
					break;
			}
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return std::nullopt;
	}

	void SemanticContext::register_variable(const std::string_view identifier, DataType type, unsigned int line_number, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_table.register_variable(identifier, type, line_number);
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_variable(identifier, type, line_number);
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				_errors.push_back(LxError(ErrorType::Internal, ss.str()));
			}
			break;
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register variable to unknown/isolated namespace";
			_errors.push_back(LxError(ErrorType::Internal, ss.str()));
		}
	}

	std::optional<FunctionSignature> SemanticContext::registered_function(const std::string_view identifier, const std::vector<DataType>& arg_types, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.registered_function(identifier, arg_types);
			
			if (ns == Namespace::Unknown || scope_depth() == 0)
			{
				if (auto ln = _global_table.registered_function(identifier, arg_types))
					return ln;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto ln = it->table.registered_function(identifier, arg_types))
					return ln;
			}
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return std::nullopt;
	}

	FunctionCallSet SemanticContext::registered_function_calls(const std::string_view identifier, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.registered_function_calls(identifier);

			FunctionCallSet callset;

			if (ns == Namespace::Unknown || scope_depth() == 0)
				callset = _global_table.registered_function_calls(identifier);

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				auto calls = it->table.registered_function_calls(identifier);
				for (const auto& call : calls)
					callset.insert(call);
			}

			return callset;
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return {};
	}

	void SemanticContext::register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_table.register_function(identifier, return_type, std::move(arg_types), line_number);
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_function(identifier, return_type, std::move(arg_types), line_number);
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				throw LxError(ErrorType::Internal, ss.str());
			}
			break;
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register function to unknown/isolated namespace";
			_errors.push_back(LxError(ErrorType::Internal, ss.str()));
		}
	}
}
