#pragma once

#include <unordered_map>

#include "util.h"
#include "types/declarations.h"

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

		CapId capture_id(const std::string_view id);
	};
}
