#include "heap.h"

namespace lx
{
	Variable DataHeap::add(DataPoint&& dp, bool temporary)
	{
		if (_free_slots.empty())
		{
			unsigned int id = _data.size();
			_data.push_back(std::move(dp));
			_unnamed.push_back({});
			return Variable(*this, id, temporary);
		}
		else
		{
			unsigned int id = _free_slots.top();
			_free_slots.pop();
			_data[id] = std::move(dp);
			_unnamed[id] = {};
			return Variable(*this, id, temporary);
		}
	}

	void DataHeap::remove(unsigned int id)
	{
		_free_slots.push(id);
		_unnamed[id].clear();
	}

	const DataPoint& DataHeap::ref(unsigned int id) const
	{
		return _data[id];
	}

	DataPoint& DataHeap::ref(unsigned int id)
	{
		return _data[id];
	}

	void DataHeap::own(unsigned int id, const Variable& var)
	{
		_unnamed[id].insert(var);
	}

	void DataHeap::disown(unsigned int id, const Variable& var)
	{
		_unnamed[id].erase(var);
	}
}
