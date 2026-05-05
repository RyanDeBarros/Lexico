#pragma once

#include "runtime.h"

#include <string_view>

namespace lx
{
	struct DataAccessor
	{
		static Variable invoke(const Variable& var, Runtime& env, const std::string_view member);
	};

	struct MethodAccessor
	{
		static Variable invoke(const Variable& var, Runtime& env, const std::string_view method, const std::vector<Variable>& args);
	};
}
