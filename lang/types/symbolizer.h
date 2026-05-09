#pragma once

#include "util.h"

namespace lx
{
	using Symbol = unsigned int;

	class Symbolizer
	{
		StringMap<Symbol> _map;

	public:
		Symbol intern(const std::string_view name);
	};
}
