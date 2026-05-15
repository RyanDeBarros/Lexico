#pragma once

#include "variable.h"

namespace lx
{
	class Iterator
	{
		Variable _iterable;
		size_t _pos = 0;

	public:
		Iterator(Variable iterable);

		bool done(const EvalContext& env) const;
		void next();

		DataPoint get(const EvalContext& env) const;
	};
}
