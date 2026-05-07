#include "datatype.h"

#include "member.h"
#include "constants.h"

namespace lx
{
	DataType::DataType(SimpleType simple)
		: _simple(simple)
	{
	}

	DataType::DataType(SimpleType simple, SimpleType underlying)
		: _simple(simple), _underlying(std::make_unique<DataType>(DataType(underlying)))
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

	std::string DataType::repr() const
	{
		switch (_simple)
		{
		case SimpleType::Int:
			return "'int'";
		case SimpleType::Float:
			return "'float'";
		case SimpleType::Bool:
			return "'bool'";
		case SimpleType::String:
			return "'string'";
		case SimpleType::Void:
			return "'void'";
		case SimpleType::Pattern:
			return "'pattern'";
		case SimpleType::Match:
			return "'match'";
		case SimpleType::Matches:
			return "'matches'";
		case SimpleType::CapId:
			return "'capid'";
		case SimpleType::Cap:
			return "'cap'";
		case SimpleType::IRange:
			return "'irange'";
		case SimpleType::SRange:
			return "'srange'";
		case SimpleType::List:
			return "'list[" + _underlying->repr() + "]'";
		default:
			return "''";
		}
	}

	SimpleType DataType::simple() const
	{
		return _simple;
	}

	const DataType& DataType::underlying() const
	{
		return *_underlying;
	}

	size_t DataType::hash() const
	{
		size_t h = std::hash<SimpleType>{}(_simple);
		if (_underlying)
			h ^= _underlying->hash() << 1; // TODO better hash function
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
			return to.simple() == SimpleType::Pattern;

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
			return to.simple() == SimpleType::Int || to.simple() == SimpleType::Float || to.simple() == SimpleType::Bool;

		case SimpleType::SRange:
			return to.simple() == SimpleType::String;

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
			return true;

		default:
			return false;
		}
	}

	static StringMap<MemberSignature> string_members()
	{
		return {
			{ "len", MemberSignature::make_data("len", DataType::Int()) },
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{.return_type = DataType::String(), .arg_types = { DataType::Int() } },
				{.return_type = DataType::String(), .arg_types = { DataType::IRange() } },
			}) },
		};
	}

	static StringMap<MemberSignature> match_members()
	{
		return {
			{ "caps", MemberSignature::make_data("caps", DataType::List(DataType::Cap())) },
			{ "start", MemberSignature::make_data("start", DataType::Int()) },
			{ "end", MemberSignature::make_data("end", DataType::Int()) },
			{ "len", MemberSignature::make_data("len", DataType::Int()) },
			{ "pos", MemberSignature::make_data("pos", DataType::IRange()) },
			{ "str", MemberSignature::make_data("str", DataType::String()) },
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{.return_type = DataType::Cap(), .arg_types = { DataType::CapId() } },
				{.return_type = DataType::Cap(), .arg_types = { DataType::Int() } },
			}) },
		};
	}

	static StringMap<MemberSignature> cap_members()
	{
		return {
			{ "exists", MemberSignature::make_data("exists", DataType::Bool()) },
			{ "start", MemberSignature::make_data("start", DataType::Int()) },
			{ "end", MemberSignature::make_data("end", DataType::Int()) },
			{ "len", MemberSignature::make_data("len", DataType::Int()) },
			{ "pos", MemberSignature::make_data("pos", DataType::IRange()) },
			{ "str", MemberSignature::make_data("str", DataType::String()) },
			{ "sub", MemberSignature::make_data("sub", DataType::Match()) },
		};
	}

	static StringMap<MemberSignature> list_common_members()
	{
		return {
			{ "len", MemberSignature::make_data("len", DataType::Int()) },
		};
	}

	static StringMap<MemberSignature> list_generic_members(const DataType& underlying)
	{
		return {
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{ .return_type = underlying, .arg_types = { DataType::Int() }},
			}) },
		};
	}

	bool DataType::member(const std::string_view name, MemberSignature& signature) const
	{
		static const std::unordered_map<SimpleType, StringMap<MemberSignature>> common_members = {
			{ SimpleType::String, string_members() },
			{ SimpleType::Match, match_members() },
			{ SimpleType::Cap, cap_members() },
			{ SimpleType::List, list_common_members() },
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
			{ SimpleType::List, &list_generic_members }
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
