#include "runtime.h"

#include "errors.h"

#include <sstream>

namespace lx
{
	std::optional<unsigned int> SymbolTable::identifier_is_registered(const std::string_view identifier) const
	{
		if (auto sig = variable_is_registered(identifier))
			return sig->decl_line_number;
		else if (auto sig = function_is_registered(identifier))
			return sig->decl_line_number;
		else
			return std::nullopt;
	}

	std::optional<VariableSignature> SymbolTable::variable_is_registered(const std::string_view identifier) const
	{
		auto it = _variable_table.find(identifier);
		if (it != _variable_table.end())
			return it->second;
		else
			return std::nullopt;
	}

	bool SymbolTable::variable_has_type(const std::string_view identifier, DataType type) const
	{
		auto it = _variable_table.find(identifier);
		if (it == _variable_table.end())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable not registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		return it->second.type == type;
	}

	void SymbolTable::register_variable(const std::string_view identifier, DataType type, unsigned int line_number)
	{
		if (_variable_table.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		_variable_table[std::string(identifier)] = { .decl_line_number = line_number, .type = type };
	}

	std::optional<FunctionSignature> SymbolTable::function_is_registered(const std::string_view identifier) const
	{
		auto it = _function_table.find(identifier);
		if (it != _function_table.end())
			return it->second;
		else
			return std::nullopt;
	}

	bool SymbolTable::function_has_return_type(const std::string_view identifier, DataType type) const
	{
		auto it = _function_table.find(identifier);
		if (it == _function_table.end())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function not registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		return it->second.return_type == type;
	}

	bool SymbolTable::function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types) const
	{
		auto it = _function_table.find(identifier);
		if (it == _function_table.end())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function not registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		return it->second.arg_types == types;
	}

	void SymbolTable::register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number)
	{
		if (_function_table.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		_function_table[std::string(identifier)] = { .decl_line_number = line_number, .return_type = return_type, .arg_types = std::move(arg_types) };
	}

	RuntimeEnvironment::RuntimeEnvironment(const std::vector<std::string_view>& script_lines)
		: _script_lines(script_lines)
	{
	}

	std::vector<LxError>& RuntimeEnvironment::errors() const
	{
		return _errors;
	}

	const std::vector<std::string_view>& RuntimeEnvironment::script_lines() const
	{
		return _script_lines;
	}

	void RuntimeEnvironment::push_local_scope(bool isolated)
	{
		_scope_stack.push_back({ .isolated = isolated });
	}

	void RuntimeEnvironment::pop_local_scope()
	{
		_scope_stack.pop_back();
	}

	std::optional<unsigned int> RuntimeEnvironment::identifier_is_registered(const std::string_view identifier, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.identifier_is_registered(identifier);
			else if (ns == Namespace::Isolated)
			{
				if (!_scope_stack.empty())
				{
					if (auto ln = _scope_stack.back().table.identifier_is_registered(identifier))
						return ln;
				}

				return std::nullopt;
			}
			else if (ns == Namespace::Unknown)
			{
				if (auto ln = _global_table.identifier_is_registered(identifier))
					return ln;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto ln = it->table.identifier_is_registered(identifier))
					return ln;
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

	std::optional<VariableSignature> RuntimeEnvironment::variable_is_registered(const std::string_view identifier, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.variable_is_registered(identifier);
			else if (ns == Namespace::Isolated)
			{
				if (!_scope_stack.empty())
				{
					if (auto sig = _scope_stack.back().table.variable_is_registered(identifier))
						return sig;
				}

				return std::nullopt;
			}
			else if (ns == Namespace::Unknown)
			{
				if (auto sig = _global_table.variable_is_registered(identifier))
					return sig;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto sig = it->table.variable_is_registered(identifier))
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

	bool RuntimeEnvironment::variable_has_type(const std::string_view identifier, DataType type, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.variable_has_type(identifier, type);
			else if (ns == Namespace::Isolated)
				return !_scope_stack.empty() && _scope_stack.back().table.variable_has_type(identifier, type);
			else if (ns == Namespace::Unknown && _global_table.variable_has_type(identifier, type))
				return true;

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (it->table.variable_has_type(identifier, type))
					return true;
				else if (it->isolated)
					break;
			}
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return false;
	}

	void RuntimeEnvironment::register_variable(const std::string_view identifier, DataType type, unsigned int line_number, Namespace ns)
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

	std::optional<FunctionSignature> RuntimeEnvironment::function_is_registered(const std::string_view identifier, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.function_is_registered(identifier);
			else if (ns == Namespace::Isolated)
			{
				if (!_scope_stack.empty())
				{
					if (auto sig = _scope_stack.back().table.function_is_registered(identifier))
						return sig;
				}

				return std::nullopt;
			}
			else if (ns == Namespace::Unknown)
			{
				if (auto ln = _global_table.function_is_registered(identifier))
					return ln;
			}

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (auto ln = it->table.function_is_registered(identifier))
					return ln;
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

	bool RuntimeEnvironment::function_has_return_type(const std::string_view identifier, DataType type, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.function_has_return_type(identifier, type);
			else if (ns == Namespace::Isolated)
				return !_scope_stack.empty() && _scope_stack.back().table.function_has_return_type(identifier, type);
			else if (ns == Namespace::Unknown && _global_table.function_has_return_type(identifier, type))
				return true;

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (it->table.function_has_return_type(identifier, type))
					return true;
				else if (it->isolated)
					break;
			}
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return false;
	}

	bool RuntimeEnvironment::function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types, Namespace ns) const
	{
		try
		{
			if (ns == Namespace::Global)
				return _global_table.function_has_arg_types(identifier, types);
			else if (ns == Namespace::Isolated)
				return !_scope_stack.empty() && _scope_stack.back().table.function_has_arg_types(identifier, types);
			else if (ns == Namespace::Unknown && _global_table.function_has_arg_types(identifier, types))
				return true;

			for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
			{
				if (it->table.function_has_arg_types(identifier, types))
					return true;
				else if (it->isolated)
					break;
			}
		}
		catch (const LxError& error)
		{
			_errors.push_back(error);
		}

		return false;
	}

	void RuntimeEnvironment::register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, unsigned int line_number, Namespace ns)
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
