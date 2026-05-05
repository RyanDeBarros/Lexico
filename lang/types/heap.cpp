#include "heap.h"

namespace lx
{
	DataPointHandle::DataPointHandle(DataHeap& heap, unsigned int id, bool temporary)
		: _heap(&heap), _id(id), _temporary(temporary), _ref_count(std::make_shared<unsigned int>(1))
	{
	}

	DataPointHandle::DataPointHandle(const DataPointHandle& other)
		: _heap(other._heap), _id(other._id), _temporary(other._temporary), _ref_count(other._ref_count)
	{
		if (_ref_count)
			++*_ref_count;
	}

	DataPointHandle::DataPointHandle(DataPointHandle&& other) noexcept
		: _heap(other._heap), _id(other._id), _temporary(other._temporary), _ref_count(std::move(other._ref_count))
	{
	}

	DataPointHandle::~DataPointHandle()
	{
		decrement();
	}

	DataPointHandle& DataPointHandle::operator=(const DataPointHandle& other)
	{
		if (this != &other)
		{
			decrement();
			_heap = other._heap;
			_id = other._id;
			_temporary = other._temporary;
			_ref_count = other._ref_count;
			if (_ref_count)
				++*_ref_count;
		}
		return *this;
	}

	DataPointHandle& DataPointHandle::operator=(DataPointHandle&& other) noexcept
	{
		if (this != &other)
		{
			decrement();
			_heap = other._heap;
			_id = other._id;
			_temporary = other._temporary;
			_ref_count = std::move(other._ref_count);
		}
		return *this;
	}

	void DataPointHandle::decrement()
	{
		if (_ref_count)
		{
			--*_ref_count;
			if (*_ref_count == 0)
			{
				_heap->remove(_id);
				_ref_count.reset();
			}
		}
	}

	const DataPoint& DataPointHandle::ref() const
	{
		if (_ref_count)
			return _heap->ref(_id);
		else
			throw LxError(ErrorType::Internal, "DataPointHandle reference count is null");
	}
	
	DataPoint& DataPointHandle::ref()
	{
		if (_ref_count)
			return _heap->ref(_id);
		else
			throw LxError(ErrorType::Internal, "DataPointHandle reference count is null");
	}

	DataPoint DataPointHandle::dp()
	{
		if (_ref_count)
		{
			if (_temporary)
			{
				if (*_ref_count != 1)
					throw LxError(ErrorType::Internal, "DataPointHandle temporary reference count is not 1");

				DataPoint dp = std::move(_heap->ref(_id));
				decrement();
				return dp;
			}
			else
				return _heap->ref(_id);
		}
		else
			throw LxError(ErrorType::Internal, "DataPointHandle reference count is null");
	}

	DataPointHandle DataHeap::add(DataPoint&& dp, bool temporary)
	{
		if (_free_slots.empty())
		{
			unsigned int id = _data.size();
			_data.push_back(std::move(dp));
			return DataPointHandle(*this, id, temporary);
		}
		else
		{
			unsigned int id = _free_slots.top();
			_free_slots.pop();
			_data[id] = std::move(dp);
			return DataPointHandle(*this, id, temporary);
		}
	}

	void DataHeap::remove(unsigned int id)
	{
		_free_slots.push(id);
	}

	const DataPoint& DataHeap::ref(unsigned int id) const
	{
		return _data[id];
	}

	DataPoint& DataHeap::ref(unsigned int id)
	{
		return _data[id];
	}
}
