#include "heap.h"

namespace lx
{
	Variable VirtualHeap::add(DataPoint&& dp)
	{
		if (_free_slots.empty())
		{
			unsigned int id = _arena.size();
			_arena.push_back(std::move(dp));
			_ref_counts.push_back(1);
			return Variable(*this, id);
		}
		else
		{
			unsigned int id = _free_slots.top();
			_free_slots.pop();
			_arena[id] = std::move(dp);
			_ref_counts[id] = 1;
			return Variable(*this, id);
		}
	}

	const DataPoint& VirtualHeap::get(unsigned int id) const
	{
		return *_arena[id];
	}

	DataPoint& VirtualHeap::get(unsigned int id)
	{
		return *_arena[id];
	}

	DataPoint VirtualHeap::detach(unsigned int id)
	{
		DataPoint dp = unbound(id) ? std::move(*_arena[id]) : *_arena[id];
		decrement_ref_count(id);
		return dp;
	}

	bool VirtualHeap::unbound(unsigned int id) const
	{
		return _ref_counts[id] == 1;
	}

	void VirtualHeap::increment_ref_count(unsigned int id)
	{
		++_ref_counts[id];
	}

	void VirtualHeap::decrement_ref_count(unsigned int id)
	{
		--_ref_counts[id];
		if (_ref_counts[id] == 0)
		{
			_free_slots.push(id);
			*_arena[id] = Void();
		}
	}
}
