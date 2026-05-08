#pragma once

#include "datapoint.h"
#include "variable.h"
#include "datapath.h"

#include <stack>

namespace lx
{
	class VirtualHeap
	{
		std::vector<DataPoint> _vars;
		std::stack<unsigned int> _free_var_slots;
		std::vector<unsigned int> _var_ref_counts;

		std::vector<DataPath> _paths;
		std::stack<unsigned int> _free_path_slots;
		std::vector<unsigned int> _path_ref_counts;

	public:
		Variable add(DataPoint&& dp);
		Variable add(DataPoint&& dp, DataPath&& path);

	private:
		friend class Variable;

		const DataPoint& get_var(unsigned int id) const;
		DataPoint& get_var(unsigned int id);
		
		const DataPath* get_path(unsigned int id) const;
		DataPath* get_path(unsigned int id);
		unsigned int new_path(DataPath&& path);
		
		DataPoint detach(unsigned int id);
		bool unbound(unsigned int id) const;

		void increment_var_ref_count(unsigned int id);
		void decrement_var_ref_count(unsigned int id);
		void increment_path_ref_count(unsigned int id);
		void decrement_path_ref_count(unsigned int id);
	};
}
