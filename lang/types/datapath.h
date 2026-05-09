#pragma once

#include "symbolizer.h"
#include "basic.h"
#include "pattern.h"

#include <vector>
#include <any>

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
