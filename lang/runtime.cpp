#include "runtime.h"

#include "types/basic.h"

namespace lx
{
	Runtime::Runtime(const std::string_view input, std::stringstream& output, std::stringstream& log)
		: _input(input), _output(output), _log(log)
	{
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
