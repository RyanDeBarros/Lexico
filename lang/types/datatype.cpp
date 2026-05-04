#include "datatype.h"

namespace lx
{
	std::string friendly_name(DataType type)
	{
		switch (type)
		{
		case DataType::Int:
			return "'int'";
		case DataType::Float:
			return "'float'";
		case DataType::Bool:
			return "'bool'";
		case DataType::String:
			return "'string'";
		case DataType::Void:
			return "'void'";
		case DataType::Pattern:
			return "'pattern'";
		case DataType::Match:
			return "'match'";
		case DataType::Matches:
			return "'matches'";
		case DataType::CapId:
			return "'capid'";
		case DataType::Cap:
			return "'cap'";
		case DataType::IRange:
			return "'irange'";
		case DataType::SRange:
			return "'srange'";
		case DataType::List:
			return "'list'";
		case DataType::_Unresolved:
		case DataType::_Marker:
		case DataType::_Scope:
		case DataType::_Color:
			return "'symbol'";
		default:
			return "''";
		}
	}

	std::ostream& operator<<(std::ostream& os, DataType type)
	{
		return os << friendly_name(type);
	}
}
