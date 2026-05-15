#include "member.h"

namespace lx
{
	MemberSignature::MemberSignature(std::string&& identifier, decltype(_layout)&& layout)
		: _identifier(std::move(identifier)), _layout(std::move(layout))
	{
	}

	const std::string& MemberSignature::identifier() const
	{
		return _identifier;
	}

	MemberSignature MemberSignature::make_data(std::string&& identifier, const DataType& type)
	{
		return MemberSignature(std::move(identifier), DataLayout{.type = type });
	}

	MemberSignature MemberSignature::make_method(std::string&& identifier, std::vector<Overload>&& overloads)
	{
		return MemberSignature(std::move(identifier), MethodLayout{.overloads = std::move(overloads) });
	}

	bool MemberSignature::is_data() const
	{
		return std::holds_alternative<DataLayout>(_layout);
	}

	bool MemberSignature::is_method() const
	{
		return std::holds_alternative<MethodLayout>(_layout);
	}

	const DataType& MemberSignature::data_type() const
	{
		return std::get<DataLayout>(_layout).type;
	}

	const std::vector<MemberSignature::Overload>& MemberSignature::method_overloads() const
	{
		return std::get<MethodLayout>(_layout).overloads;
	}

	std::optional<DataType> MemberSignature::return_type(const std::vector<DataType>& arg_types) const
	{
		const auto& overloads = method_overloads();
		for (const auto& overload : overloads)
			if (overload.arg_types == arg_types)
				return overload.return_type;

		return std::nullopt;
	}
}
