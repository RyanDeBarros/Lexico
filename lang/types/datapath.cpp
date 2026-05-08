#include "datapath.h"

namespace lx
{
	DataSymbol DataSymbolTable::intern(const std::string_view name)
	{
		auto it = _map.find(name);
		if (it != _map.end())
			return it->second;
		else
			return _map.try_emplace(std::string(name), DataSymbol(_map.size())).first->second;
	}
}
