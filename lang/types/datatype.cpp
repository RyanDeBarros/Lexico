#include "datatype.h"

#include "member.h"
#include "constants.h"
#include "errors.h"
#include "primitives/include.h"

namespace lx
{
	DataType::DataType(SimpleType simple)
		: _simple(simple)
	{
	}

	DataType::DataType(SimpleType simple, const DataType& underlying)
		: _simple(simple), _underlying(std::make_unique<DataType>(DataType(underlying)))
	{
	}

	DataType::DataType(SimpleType simple, DataType&& underlying)
		: _simple(simple), _underlying(std::make_unique<DataType>(std::move(underlying)))
	{
	}

	DataType::DataType(const DataType& other)
		: _simple(other._simple), _underlying(other._underlying ? std::make_unique<DataType>(*other._underlying) : nullptr)
	{
	}

	DataType& DataType::operator=(const DataType& other)
	{
		if (this != &other)
		{
			_simple = other._simple;
			if (other._underlying)
			{
				if (_underlying)
					*_underlying = *other._underlying;
				else
					_underlying = std::make_unique<DataType>(*other._underlying);
			}
			else
				_underlying.reset();
		}
		return *this;
	}

	DataType DataType::Int()
	{
		return DataType(SimpleType::Int);
	}
	
	DataType DataType::Float()
	{
		return DataType(SimpleType::Float);
	}
	
	DataType DataType::Bool()
	{
		return DataType(SimpleType::Bool);
	}
	
	DataType DataType::String()
	{
		return DataType(SimpleType::String);
	}

	DataType DataType::StringView()
	{
		return DataType(SimpleType::StringView);
	}

	DataType DataType::Void()
	{
		return DataType(SimpleType::Void);
	}
	
	DataType DataType::Pattern()
	{
		return DataType(SimpleType::Pattern);
	}
	
	DataType DataType::Match()
	{
		return DataType(SimpleType::Match);
	}
	
	DataType DataType::Matches()
	{
		return DataType(SimpleType::Matches);
	}
	
	DataType DataType::CapId()
	{
		return DataType(SimpleType::CapId);
	}
	
	DataType DataType::Cap()
	{
		return DataType(SimpleType::Cap);
	}
	
	DataType DataType::IRange()
	{
		return DataType(SimpleType::IRange);
	}
	
	DataType DataType::SRange()
	{
		return DataType(SimpleType::SRange);
	}

	DataType DataType::List(const DataType& underlying)
	{
		return DataType(SimpleType::List, underlying);
	}

	DataType DataType::List(DataType&& underlying)
	{
		return DataType(SimpleType::List, std::move(underlying));
	}

	std::string DataType::repr(bool delimit) const
	{
		std::string repr;
		if (delimit)
			repr += "'";
		switch (_simple)
		{
		case SimpleType::Int:
			repr += "int";
			break;
		case SimpleType::Float:
			repr += "float";
			break;
		case SimpleType::Bool:
			repr += "bool";
			break;
		case SimpleType::String:
			repr += "string";
			break;
		case SimpleType::Void:
			repr += "void";
			break;
		case SimpleType::Pattern:
			repr += "pattern";
			break;
		case SimpleType::Match:
			repr += "match";
			break;
		case SimpleType::Matches:
			repr += "matches";
			break;
		case SimpleType::CapId:
			repr += "capid";
			break;
		case SimpleType::Cap:
			repr += "cap";
			break;
		case SimpleType::IRange:
			repr += "irange";
			break;
		case SimpleType::SRange:
			repr += "srange";
			break;
		case SimpleType::List:
			repr += "list[" + _underlying->repr(false) + "]";
			break;
		}
		if (delimit)
			repr += "'";
		return repr;
	}

	SimpleType DataType::simple() const
	{
		return _simple;
	}

	const DataType& DataType::underlying(const ScriptSegment* segment) const
	{
		if (_underlying)
			return *_underlying;
		else
		{
			std::string err = "underlying type is null for simple type " + std::to_string(static_cast<int>(_simple));
			if (segment)
				throw LxError::segment_error(*segment, ErrorType::Internal, err);
			else
				throw LxError(ErrorType::Internal, std::move(err));
		}
	}

	size_t DataType::hash() const
	{
		size_t h = static_cast<size_t>(_simple);
		const DataType* cur = _underlying.get();
		while (cur)
		{
			h = (h << 3) ^ 0xA;
			h ^= static_cast<size_t>(cur->_simple);
			cur = cur->_underlying.get();
		}
		return h;
	}

	bool DataType::operator==(const DataType& other) const
	{
		if (_simple != other._simple)
			return false;
		else if (_underlying)
			return other._underlying && *_underlying == *other._underlying;
		else
			return !other._underlying;
	}

	bool DataType::can_cast_implicit(const DataType& to) const
	{
		if (to.simple() == SimpleType::Void || *this == to)
			return true;

		switch (_simple)
		{
		case SimpleType::Int:
			return to.simple() == SimpleType::Float || to.simple() == SimpleType::Bool || to.simple() == SimpleType::IRange;

		case SimpleType::Float:
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Bool;

		case SimpleType::Bool:
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Float;

		case SimpleType::String:
			return to.simple() == SimpleType::Pattern || to.simple() == SimpleType::StringView;

		case SimpleType::StringView:
			return to.simple() == SimpleType::Pattern || to.simple() == SimpleType::String;

		case SimpleType::SRange:
			return to.simple() == SimpleType::Pattern;

		default:
			return false;
		}
	}

	bool DataType::can_cast_explicit(const DataType& to) const
	{
		if (can_cast_implicit(to))
			return true;

		switch (_simple)
		{
		case SimpleType::Int:
		case SimpleType::Float:
			return to.simple() == SimpleType::String || to.simple() == SimpleType::Pattern;

		case SimpleType::Bool:
			return to.simple() == SimpleType::String;

		case SimpleType::String:
		case SimpleType::StringView:
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Float || to.simple() == SimpleType::Bool;

		case SimpleType::SRange:
			return to.simple() == SimpleType::String;

		case SimpleType::Matches:
			return to == DataType::List(DataType::Match());

		default:
			return false;
		}
	}

	bool DataType::is_view() const
	{
		switch (_simple)
		{
		case SimpleType::StringView:
			return true;

		default:
			return false;
		}
	}

	bool DataType::is_iterable() const
	{
		switch (_simple)
		{
		case SimpleType::String:
		case SimpleType::Match:
		case SimpleType::Matches:
		case SimpleType::IRange:
		case SimpleType::SRange:
		case SimpleType::List:
			return true;

		default:
			return false;
		}
	}

	std::optional<DataType> DataType::itertype() const
	{
		switch (_simple)
		{
		case SimpleType::String:
			return DataType::String();

		case SimpleType::Match:
			return DataType::Cap();

		case SimpleType::Matches:
			return DataType::Match();

		case SimpleType::IRange:
			return DataType::Int();

		case SimpleType::SRange:
			return DataType::String();

		case SimpleType::List:
			return *_underlying;

		default:
			return std::nullopt;
		}
	}

	bool DataType::is_highlightable() const
	{
		switch (_simple)
		{
		case SimpleType::Int:
		case SimpleType::Match:
		case SimpleType::Matches:
		case SimpleType::IRange:
			return true;

		default:
			return false;
		}
	}

	bool DataType::is_pageable() const
	{
		switch (_simple)
		{
		case SimpleType::String:
		case SimpleType::StringView:
			return true;

		default:
			return false;
		}
	}

	bool DataType::member(const std::string_view name, MemberSignature& signature) const
	{
		static const std::unordered_map<SimpleType, StringMap<MemberSignature>> common_members = {
			{ SimpleType::String, String::members() },
			{ SimpleType::StringView, StringView::members() },
			{ SimpleType::Match, Match::members() },
			{ SimpleType::Matches, Matches::members() },
			{ SimpleType::Cap, Cap::members() },
			{ SimpleType::List, List::members() },
		};

		auto it = common_members.find(_simple);
		if (it != common_members.end())
		{
			auto subit = it->second.find(name);
			if (subit != it->second.end())
			{
				signature = subit->second;
				return true;
			}
		}

		static std::unordered_map<SimpleType, std::unordered_map<DataType, StringMap<MemberSignature>>> generic_members = {
			{ SimpleType::List, {} }
		};

		static const std::unordered_map<SimpleType, StringMap<MemberSignature>(*)(const DataType&)> generic_generators = {
			{ SimpleType::List, &List::members }
		};
		
		if (_underlying)
		{
			auto it = generic_members.find(_simple);
			if (it != generic_members.end())
			{
				auto uit = it->second.find(*_underlying);
				if (uit == it->second.end())
				{
					auto gen = generic_generators.find(_simple);
					if (gen != generic_generators.end())
						uit = it->second.emplace(*_underlying, gen->second(*_underlying)).first;
				}

				if (uit != it->second.end())
				{
					auto subit = uit->second.find(name);
					if (subit != uit->second.end())
					{
						signature = subit->second;
						return true;
					}
				}
			}
		}

		return false;
	}

	std::ostream& operator<<(std::ostream& os, const DataType& type)
	{
		return os << type.repr();
	}
}

size_t std::hash<lx::DataType>::operator()(const lx::DataType& d) const
{
	return d.hash();
}
