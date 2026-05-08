#pragma once

#include <vector>

#include "util.h"

namespace lx
{
	using DataSymbol = unsigned int;

	class DataSymbolTable
	{
		StringMap<DataSymbol> _map;

	public:
		DataSymbol intern(const std::string_view name);
	};

	struct PathStep
	{
		DataSymbol symbol;
		int index;
	};

	struct DataPath
	{
		std::vector<PathStep> steps;
	};
}
