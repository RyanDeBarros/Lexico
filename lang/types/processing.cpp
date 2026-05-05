#include "processing.h"

namespace lx
{
	Variable operate(Runtime& env, BinaryOperator op, const Variable& lhs, const Variable& rhs)
	{
		// TODO note that '=' shouldn't allow for assigning to temporary Variables
		return env.temporary_variable(Void());
	}

	Variable operate(Runtime& env, PrefixOperator op, const Variable& var)
	{
		// TODO
		return env.temporary_variable(Void());
	}

	Variable operate(Runtime& env, PatternSimpleRepeatOperator op, const Variable& var)
	{
		// TODO
		return env.temporary_variable(Void());
	}
}
