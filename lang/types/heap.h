#pragma once

#include "datapoint.h"

#include <stack>

namespace lx
{
	class DataHeap;

	class DataPointHandle
	{
		DataHeap* _heap;
		unsigned int _id;
		bool _temporary;
		std::shared_ptr<unsigned int> _ref_count;

	public:
		DataPointHandle(DataHeap& heap, unsigned int id, bool temporary);
		DataPointHandle(const DataPointHandle&);
		DataPointHandle(DataPointHandle&&) noexcept;
		~DataPointHandle();
		DataPointHandle& operator=(const DataPointHandle&);
		DataPointHandle& operator=(DataPointHandle&&) noexcept;

	private:
		void decrement();

	public:
		const DataPoint& ref() const;
		DataPoint& ref();
		DataPoint dp();
	};

	class DataHeap
	{
		std::vector<DataPoint> _data;
		std::stack<unsigned int> _free_slots;

	public:
		DataPointHandle add(DataPoint&& dp, bool temporary);
		void remove(unsigned int id);
		const DataPoint& ref(unsigned int id) const;
		DataPoint& ref(unsigned int id);
	};
}
