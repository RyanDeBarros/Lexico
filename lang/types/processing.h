#pragma once

#include "operations.h"
#include "variable.h"
#include "runtime.h"

namespace lx
{
	extern Variable operate(Runtime& env, BinaryOperator op, Variable lhs, const Variable& rhs);
	extern Variable operate(Runtime& env, PrefixOperator op, Variable var);
	extern Variable operate(Runtime& env, PatternSimpleRepeatOperator op, Variable var, const ScriptSegment& var_segment);
}
