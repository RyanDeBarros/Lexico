#pragma once

#include "token.h"

namespace lx
{
	class Parser
	{
	public:
		void parse(TokenStream& stream);
	};
}
