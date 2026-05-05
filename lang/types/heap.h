#pragma once

#include "datapoint.h"
#include "variable.h"

#include <stack>

namespace lx
{
	class DataHeap
	{
		std::vector<DataPoint> _data;
		std::vector<std::unordered_set<Variable>> _unnamed;
		std::stack<unsigned int> _free_slots;

	public:
		Variable add(DataPoint&& dp, bool temporary);

	private:
		friend class Variable;
		void remove(unsigned int id);
		const DataPoint& ref(unsigned int id) const;
		DataPoint& ref(unsigned int id);
		void own(unsigned int id, const Variable& var);
		void disown(unsigned int id, const Variable& var);
	};
}
