#pragma once

#include "declarations.h"
#include "pattern.h"

namespace lx
{
	class Unresolved
	{
		PublicTypeVariant _v;

	public:
		Unresolved(const PublicTypeVariant& v);
		Unresolved(PublicTypeVariant&& v);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
		void print(std::stringstream& ss) const;
	};
}
