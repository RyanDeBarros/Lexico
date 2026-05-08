#pragma once

#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace lx
{
	enum class SimpleType
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
	};

	class MemberSignature;

	class DataType
	{
		SimpleType _simple;
		std::unique_ptr<DataType> _underlying;

		explicit DataType(SimpleType simple);
		DataType(SimpleType simple, const DataType& underlying);
		DataType(SimpleType simple, DataType&& underlying);

	public:
		DataType(const DataType& other);
		DataType(DataType&& other) noexcept = default;
		DataType& operator=(const DataType& other);
		DataType& operator=(DataType&& other) noexcept = default;

		static DataType Int();
		static DataType Float();
		static DataType Bool();
		static DataType String();
		static DataType Void();
		static DataType Pattern();
		static DataType Match();
		static DataType Matches();
		static DataType CapId();
		static DataType Cap();
		static DataType IRange();
		static DataType SRange();
		static DataType List(const DataType& underlying);
		static DataType List(DataType&& underlying);

		std::string repr() const;
		SimpleType simple() const;
		const DataType& underlying() const;
		size_t hash() const;
		bool operator==(const DataType& other) const;

		bool can_cast_implicit(const DataType& to) const;
		bool can_cast_explicit(const DataType& to) const;
		bool is_iterable() const;
		std::optional<DataType> itertype() const;
		bool is_highlightable() const;
		bool is_pageable() const;

		bool member(const std::string_view name, MemberSignature& signature) const;
	};

	extern std::ostream& operator<<(std::ostream& os, const DataType& type);
}

template<>
struct std::hash<lx::DataType>
{
	size_t operator()(const lx::DataType&) const;
};
