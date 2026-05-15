#pragma once

#include "datatype.h"
#include "util.h"

#include <optional>
#include <variant>

namespace lx
{
	class MemberSignature
	{
		struct DataLayout
		{
			DataType type;
		};

		struct Overload
		{
			DataType return_type;
			std::vector<DataType> arg_types;
		};

		struct MethodLayout
		{
			std::vector<Overload> overloads;
		};

		std::string _identifier;
		std::variant<std::monostate, DataLayout, MethodLayout> _layout;

		MemberSignature(std::string&& identifier, decltype(_layout)&& layout);

	public:
		MemberSignature() = default;

		const std::string& identifier() const;

		static MemberSignature make_data(std::string&& identifier, const DataType& type);
		static MemberSignature make_method(std::string&& identifier, std::vector<Overload>&& overloads);

		bool is_data() const;
		bool is_method() const;

		const DataType& data_type() const;
		const std::vector<Overload>& method_overloads() const;
		std::optional<DataType> return_type(const std::vector<DataType>& arg_types) const;
	};
}
