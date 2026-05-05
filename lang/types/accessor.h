#pragma once

#include "runtime.h"

#include <string_view>

namespace lx
{
	struct DataAccessor
	{
		Variable invoke(const Variable& var, const Runtime& env, const std::string_view member);
	};

	struct MethodAccessor
	{
		Variable invoke(const Variable& var, const Runtime& env, const std::string_view method, const std::vector<Variable>& args);
	};
}
