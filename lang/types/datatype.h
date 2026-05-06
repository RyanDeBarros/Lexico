#pragma once

#include <ostream>
#include <string>

namespace lx
{
	enum class DataType
	{
		Int,
		Float,
		Bool,
		String,
		Void,
		Pattern,
		Match,
		Matches,
		CapId,
		Cap,
		IRange,
		SRange,
		List,

		// Internal
		_Unresolved,
	};

	extern std::string friendly_name(DataType type);

	extern std::ostream& operator<<(std::ostream& os, DataType type);
}
