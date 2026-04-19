#include "lang.hpp"

namespace lx
{
	bool execute(const std::string_view input, std::string& output, std::string& log)
	{
		output = input;
		log = "success!";
		return true;
	}
}
