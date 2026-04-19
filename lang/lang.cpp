#include "lang.h"

namespace lx
{
	bool execute(const std::string_view script, const std::string_view input, std::string& output, std::string& log)
	{
		output = input;
		log = "success!";
		return true;
	}
}
