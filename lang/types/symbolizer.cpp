#include "symbolizer.h"

namespace lx
{
	Symbol Symbolizer::intern(const std::string_view name)
	{
		auto it = _map.find(name);
		if (it != _map.end())
			return it->second;
		else
			return _map.try_emplace(std::string(name), Symbol(_map.size())).first->second;
	}
}
