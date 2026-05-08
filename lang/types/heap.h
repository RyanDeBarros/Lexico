#pragma once

#include "datapoint.h"
#include "variable.h"

#include <stack>
#include <unordered_set>

namespace lx
{
	class VirtualHeap
	{
		std::vector<DataPoint> _data;
		std::stack<unsigned int> _free_slots;
		std::vector<unsigned int> _ref_counts;

	public:
		Variable add(DataPoint&& dp);

	private:
		friend class Variable;

		void remove(unsigned int id);
		const DataPoint& get(unsigned int id) const;
		DataPoint& get(unsigned int id);
		DataPoint dp(unsigned int id);
		bool unbound(unsigned int id) const;

		void increment_ref_count(unsigned int id);
		void decrement_ref_count(unsigned int id);
	};
}
