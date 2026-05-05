#pragma once

#include "operations.h"
#include "variable.h"
#include "runtime.h"

namespace lx
{
	extern Variable operate(Runtime& env, BinaryOperator op, const Variable& lhs, const Variable& rhs);
	extern Variable operate(Runtime& env, PrefixOperator op, const Variable& var);
	extern Variable operate(Runtime& env, PatternSimpleRepeatOperator op, const Variable& var);
}
