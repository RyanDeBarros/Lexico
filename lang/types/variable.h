#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace lx
{
	class DataPoint;
	class VirtualHeap;
	class Runtime;
	struct ScriptSegment;

	class Variable
	{
		VirtualHeap* _heap;
		unsigned int _id;

		friend class VirtualHeap;
		Variable(VirtualHeap& heap, unsigned int id);

	public:
		Variable(const Variable&);
		Variable(Variable&&) noexcept;
		~Variable();
		Variable& operator=(const Variable&);
		Variable& operator=(Variable&&) noexcept;

		const DataPoint& ref() const;
		DataPoint& ref();
		DataPoint consume() &&;

		template<typename T>
		T consume_as() &&;

		bool unbound() const;

		size_t hash() const;
		bool operator==(const Variable&) const = default;

		Variable data_member(Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
	};
}

template<>
struct std::hash<lx::Variable>
{
	size_t operator()(const lx::Variable& var) const;
};
