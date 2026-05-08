#pragma once

#include "operations.h"
#include "variable.h"
#include "runtime.h"

namespace lx
{
	extern Variable operate(EvalContext& env, BinaryOperator op, Variable&& lhs, Variable&& rhs);
	extern Variable operate(EvalContext& env, PrefixOperator op, Variable&& var);
	extern Variable operate(EvalContext& env, PatternSimpleRepeatOperator op, Variable&& var);
}
