#pragma once

#include "operations.h"
#include "variable.h"
#include "runtime.h"

namespace lx
{
	extern Variable operate(const EvalContext& env, BinaryOperator op, Variable&& lhs, Variable&& rhs);
	extern Variable operate(const EvalContext& env, PrefixOperator op, Variable&& var);
	extern Variable operate(const EvalContext& env, PatternSimpleRepeatOperator op, Variable&& var);
}
