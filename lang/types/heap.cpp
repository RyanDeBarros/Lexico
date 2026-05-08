#include "heap.h"

namespace lx
{
	static bool is_valid_path(unsigned int pid)
	{
		return pid > 0;
	}

	static unsigned int path_index(unsigned int pid)
	{
		return pid - 1;
	}

	Variable VirtualHeap::add(DataPoint&& dp)
	{
		if (_free_var_slots.empty())
		{
			unsigned int id = _vars.size();
			_vars.push_back(std::move(dp));
			_var_ref_counts.push_back(1);
			return Variable(*this, id);
		}
		else
		{
			unsigned int id = _free_var_slots.top();
			_free_var_slots.pop();
			_vars[id] = std::move(dp);
			_var_ref_counts[id] = 1;
			return Variable(*this, id);
		}
	}

	Variable VirtualHeap::add(DataPoint&& dp, DataPath&& path)
	{
		if (_free_var_slots.empty())
		{
			unsigned int id = _vars.size();
			_vars.push_back(std::move(dp));
			_var_ref_counts.push_back(1);
			return Variable(*this, id, new_path(std::move(path)));
		}
		else
		{
			unsigned int id = _free_var_slots.top();
			_free_var_slots.pop();
			_vars[id] = std::move(dp);
			_var_ref_counts[id] = 1;
			return Variable(*this, id, new_path(std::move(path)));
		}
	}

	const DataPoint& VirtualHeap::get_var(unsigned int id) const
	{
		return _vars[id];
	}

	DataPoint& VirtualHeap::get_var(unsigned int id)
	{
		return _vars[id];
	}

	const DataPath* VirtualHeap::get_path(unsigned int id) const
	{
		if (is_valid_path(id))
			return &_paths[path_index(id)];
		else
			return nullptr;
	}

	DataPath* VirtualHeap::get_path(unsigned int id)
	{
		if (is_valid_path(id))
			return &_paths[path_index(id)];
		else
			return nullptr;
	}

	unsigned int VirtualHeap::new_path(DataPath&& path)
	{
		if (_free_path_slots.empty())
		{
			unsigned int pid = _paths.size() + 1;
			_paths.push_back(std::move(path));
			_path_ref_counts.push_back(1);
			return pid;
		}
		else
		{
			unsigned int pid = _free_path_slots.top();
			_free_path_slots.pop();
			_paths[path_index(pid)] = std::move(path);
			_path_ref_counts[path_index(pid)] = 1;
			return pid;
		}
	}

	DataPoint VirtualHeap::detach(unsigned int id)
	{
		DataPoint dp = unbound(id) ? std::move(_vars[id]) : _vars[id];
		decrement_var_ref_count(id);
		return dp;
	}

	bool VirtualHeap::unbound(unsigned int id) const
	{
		return _var_ref_counts[id] == 1;
	}

	void VirtualHeap::increment_var_ref_count(unsigned int id)
	{
		++_var_ref_counts[id];
	}

	void VirtualHeap::decrement_var_ref_count(unsigned int id)
	{
		--_var_ref_counts[id];
		if (_var_ref_counts[id] == 0)
		{
			_free_var_slots.push(id);
			_vars[id] = Void();
		}
	}

	void VirtualHeap::increment_path_ref_count(unsigned int id)
	{
		if (is_valid_path(id)) [[unlikely]]
			++_path_ref_counts[path_index(id)];
	}

	void VirtualHeap::decrement_path_ref_count(unsigned int id)
	{
		if (is_valid_path(id)) [[unlikely]]
		{
			--_path_ref_counts[path_index(id)];
			if (_path_ref_counts[path_index(id)] == 0)
			{
				_free_path_slots.push(path_index(id));
				_paths[path_index(id)] = {};
			}
		}
	}
}
