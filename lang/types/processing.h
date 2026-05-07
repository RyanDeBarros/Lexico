#pragma once

#include "operations.h"
#include "variable.h"
#include "runtime.h"

namespace lx
{
	extern Variable operate(Runtime& env, BinaryOperator op, Variable&& lhs, const ScriptSegment& lhs_segment, Variable&& rhs, const ScriptSegment& rhs_segment);
	extern Variable operate(Runtime& env, PrefixOperator op, Variable&& var, const ScriptSegment& segment);
	extern Variable operate(Runtime& env, PatternSimpleRepeatOperator op, Variable&& var, const ScriptSegment& var_segment);
}
