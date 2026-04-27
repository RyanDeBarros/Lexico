#include "runtime.h"

#include "errors.h"

#include <sstream>

namespace lx
{
	bool SymbolTable::variable_is_registered(const std::string_view identifier) const
	{
		return _variable_table.count(identifier);
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

	void SymbolTable::register_variable(const std::string_view identifier, DataType type)
	{
		if (_variable_table.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		_variable_table[std::string(identifier)] = { .type = type };
	}

	bool SymbolTable::function_is_registered(const std::string_view identifier) const
	{
		return _function_table.count(identifier);
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

	void SymbolTable::register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types)
	{
		if (_function_table.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": function already registered: " << identifier;
			throw LxError(ErrorType::Internal, ss.str());
		}

		_function_table[std::string(identifier)] = { .return_type = return_type, .arg_types = std::move(arg_types) };
	}

	void RuntimeEnvironment::push_local_scope(bool isolated)
	{
		_scope_stack.push_back({ .isolated = isolated });
	}

	void RuntimeEnvironment::pop_local_scope()
	{
		_scope_stack.pop_back();
	}

	bool RuntimeEnvironment::variable_is_registered(const std::string_view identifier, Namespace ns) const
	{
		if (ns == Namespace::Global)
			return _global_table.variable_is_registered(identifier);
		else if (ns == Namespace::Unknown && _global_table.variable_is_registered(identifier))
			return true;

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (it->table.variable_is_registered(identifier))
				return true;
			else if (it->isolated)
				break;
		}

		return false;
	}

	bool RuntimeEnvironment::variable_has_type(const std::string_view identifier, DataType type, Namespace ns) const
	{
		if (ns == Namespace::Global)
			return _global_table.variable_has_type(identifier, type);
		else if (ns == Namespace::Unknown && _global_table.variable_has_type(identifier, type))
			return true;

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (it->table.variable_has_type(identifier, type))
				return true;
			else if (it->isolated)
				break;
		}

		return false;
	}

	void RuntimeEnvironment::register_variable(const std::string_view identifier, DataType type, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_table.register_variable(identifier, type);
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_variable(identifier, type);
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				throw LxError(ErrorType::Internal, ss.str());
			}
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register variable to unknown namespace";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	bool RuntimeEnvironment::function_is_registered(const std::string_view identifier, Namespace ns) const
	{
		if (ns == Namespace::Global)
			return _global_table.function_is_registered(identifier);
		else if (ns == Namespace::Unknown && _global_table.function_is_registered(identifier))
			return true;

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (it->table.function_is_registered(identifier))
				return true;
			else if (it->isolated)
				break;
		}

		return false;
	}

	bool RuntimeEnvironment::function_has_return_type(const std::string_view identifier, DataType type, Namespace ns) const
	{
		if (ns == Namespace::Global)
			return _global_table.function_has_return_type(identifier, type);
		else if (ns == Namespace::Unknown && _global_table.function_has_return_type(identifier, type))
			return true;

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (it->table.function_has_return_type(identifier, type))
				return true;
			else if (it->isolated)
				break;
		}

		return false;
	}

	bool RuntimeEnvironment::function_has_arg_types(const std::string_view identifier, const std::vector<DataType>& types, Namespace ns) const
	{
		if (ns == Namespace::Global)
			return _global_table.function_has_arg_types(identifier, types);
		else if (ns == Namespace::Unknown && _global_table.function_has_arg_types(identifier, types))
			return true;

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (it->table.function_has_arg_types(identifier, types))
				return true;
			else if (it->isolated)
				break;
		}

		return false;
	}

	void RuntimeEnvironment::register_function(const std::string_view identifier, DataType return_type, std::vector<DataType>&& arg_types, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_table.register_function(identifier, return_type, std::move(arg_types));
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_function(identifier, return_type, std::move(arg_types));
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				throw LxError(ErrorType::Internal, ss.str());
			}
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register function to unknown namespace";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}
}
