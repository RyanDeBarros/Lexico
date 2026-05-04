#pragma once

#include <unordered_map>

#include "util.h"
#include "namespace.h"
#include "types/datapoint.h"

namespace lx
{
	class Runtime
	{
		const std::string_view _input;
		std::stringstream& _output;
		std::stringstream& _log;

		StringMap<unsigned int> _capture_ids;

	public:
		Runtime(const std::string_view input, std::stringstream& output, std::stringstream& log);

		void push_local_scope(bool isolated);
		void pop_local_scope();

		void register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns);
		const DataPoint& registered_variable(const std::string_view identifier, Namespace ns) const;
		DataPoint& registered_variable(const std::string_view identifier, Namespace ns);

		CapId capture_id(const std::string_view id);
	};
}
