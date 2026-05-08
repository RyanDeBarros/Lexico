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
		// TODO use black box & user pointer + index instead of int. For example, [] might use int indexes or irange indexes
		int index;
	};

	struct DataPath
	{
		std::vector<PathStep> steps;
	};
}
