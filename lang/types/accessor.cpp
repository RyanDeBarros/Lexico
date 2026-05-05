#include "accessor.h"

namespace lx
{
	Variable DataAccessor::invoke(const Variable& var, const Runtime& env, const std::string_view member)
	{
		// TODO
		return env.temporary_variable(Void());
	}

	Variable MethodAccessor::invoke(const Variable& var, const Runtime& env, const std::string_view method, const std::vector<Variable>& args)
	{
		// TODO
		return env.temporary_variable(Void());
	}
}
