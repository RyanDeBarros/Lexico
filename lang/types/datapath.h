#pragma once

#include "symbolizer.h"
#include "primitives/include.h"

#include <vector>

namespace lx
{
	struct PathStep
	{
		Symbol symbol;
		DataPoint aux = Void();
	};

	struct DataPath
	{
		std::vector<PathStep> steps;
	};
}
