#include "runtime.h"

namespace lx
{
	void RuntimeSymbolTable::register_variable(const std::string_view identifier, Variable&& dp)
	{
		if (_variable_table.count(identifier))
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": variable already registered: " << identifier;
			throw LxError(ErrorType::Runtime, ss.str());
		}

		_variable_table.try_emplace(std::string(identifier), std::move(dp));
	}

	std::optional<Variable> RuntimeSymbolTable::registered_variable(const std::string_view identifier) const
	{
		auto it = _variable_table.find(identifier);
		if (it != _variable_table.end())
			return it->second;
		else
			return std::nullopt;
	}

	RuntimeScopeContext::RuntimeScopeContext(bool isolated)
		: isolated(isolated)
	{
	}

	Runtime::Runtime(const std::string_view input)
		: _input(input), _global_matches(_heap.add(Matches(), false)), _search_scope(std::nullopt)
	{
	}

	const std::stringstream& Runtime::output() const
	{
		return _output;
	}

	std::stringstream& Runtime::output()
	{
		return _output;
	}

	const std::stringstream& Runtime::log() const
	{
		return _log;
	}

	std::stringstream& Runtime::log()
	{
		return _log;
	}

	void Runtime::push_local_scope(bool isolated)
	{
		_scope_stack.emplace_back(isolated);
	}

	void Runtime::pop_local_scope()
	{
		if (_scope_stack.empty())
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": scope stack is empty";
			throw LxError(ErrorType::Internal, ss.str());
		}
		_scope_stack.pop_back();
	}

	unsigned int Runtime::scope_depth() const
	{
		return _scope_stack.size();
	}

	void Runtime::register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns)
	{
		switch (ns)
		{
		case lx::Namespace::Global:
			_global_table.register_variable(identifier, _heap.add(std::move(dp), false));
			break;
		case lx::Namespace::Local:
			if (!_scope_stack.empty())
				_scope_stack.back().table.register_variable(identifier, _heap.add(std::move(dp), false));
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": local scope stack is empty";
				throw LxError(ErrorType::Runtime, ss.str());
			}
			break;
		default:
			std::stringstream ss;
			ss << __FUNCTION__ << ": cannot register variable to unknown/isolated namespace";
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	Variable Runtime::registered_variable(const std::string_view identifier, Namespace ns) const
	{
		if (ns == Namespace::Global)
		{
			if (auto dp = _global_table.registered_variable(identifier))
				return *dp;
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": global variable \"" << identifier << "\" not present in global table";
				throw LxError(ErrorType::Runtime, ss.str());
			}
		}

		if (ns == Namespace::Unknown || scope_depth() == 0)
		{
			if (auto sig = _global_table.registered_variable(identifier))
				return *sig;
		}

		for (auto it = _scope_stack.rbegin(); it != _scope_stack.rend(); ++it)
		{
			if (auto sig = it->table.registered_variable(identifier))
				return *sig;
			else if (it->isolated)
				break;
		}

		std::stringstream ss;
		ss << __FUNCTION__ << ": variable \"" << identifier << "\" not present in any table";
		throw LxError(ErrorType::Runtime, ss.str());
	}

	Variable Runtime::temporary_variable(DataPoint&& dp) const
	{
		return _heap.add(std::move(dp), true);
	}

	Variable Runtime::unnamed_variable(Variable& owner, DataPoint&& dp) const
	{
		auto var = _heap.add(std::move(dp), false);
		owner.own(var);
		return var;
	}

	const Matches& Runtime::global_matches() const
	{
		return _global_matches.ref().get<Matches>();
	}

	Matches& Runtime::global_matches()
	{
		return _global_matches.ref().get<Matches>();
	}

	Variable Runtime::global_matches_handle() const
	{
		return _global_matches;
	}

	// TODO use search scope in find execution

	const Scope& Runtime::search_scope() const
	{
		return _search_scope;
	}

	Scope& Runtime::search_scope()
	{
		return _search_scope;
	}

	CapId Runtime::capture_id(const std::string_view id)
	{
		auto it = _capture_ids.find(id);
		if (it != _capture_ids.end())
			return CapId(it->second);

		unsigned int uid = _capture_ids.size();
		_capture_ids[std::string(id)] = uid;
		return CapId(uid);
	}
}
