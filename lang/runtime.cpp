#include "runtime.h"

#include "types/basic.h"

namespace lx
{
	Runtime::Runtime(const std::string_view input, std::stringstream& output, std::stringstream& log)
		: _input(input), _output(output), _log(log)
	{
	}

	void Runtime::push_local_scope(bool isolated)
	{
		// TODO
	}

	void Runtime::pop_local_scope()
	{
		// TODO
	}

	void Runtime::register_variable(const std::string_view identifier, DataPoint&& dp, Namespace ns)
	{
		// TODO
	}

	const DataPoint& Runtime::registered_variable(const std::string_view identifier, Namespace ns) const
	{
		// TODO
	}

	DataPoint& Runtime::registered_variable(const std::string_view identifier, Namespace ns)
	{
		// TODO
	}

	CapId Runtime::capture_id(const std::string_view id)
	{
		auto it = _capture_ids.find(id);
		if (it != _capture_ids.end())
			return CapId(it->second);

		unsigned int uid = _capture_ids.size();
		_capture_ids[std::string(id)] = uid;
		return CapId(uid);
	}
}
