#pragma once

#include "datapoint.h"
#include "variable.h"

#include <stack>
#include <unordered_set>

namespace lx
{
	class DataHeap
	{
		std::vector<DataPoint> _data;
		std::stack<unsigned int> _free_slots;

	public:
		Variable add(DataPoint&& dp, bool temporary);

	private:
		friend class Variable;
		void remove(unsigned int id);
		const DataPoint& ref(unsigned int id) const;
		DataPoint& ref(unsigned int id);
	};
}
