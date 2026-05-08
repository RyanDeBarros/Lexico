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
	struct DataPath;

	class Variable
	{
		VirtualHeap* _heap = nullptr;
		unsigned int _id;
		unsigned int _path = 0;

		friend class VirtualHeap;
		Variable(VirtualHeap& heap, unsigned int id);
		Variable(VirtualHeap& heap, unsigned int id, unsigned int path);

	public:
		Variable(const Variable&);
		Variable(Variable&&) noexcept;
		~Variable();
		Variable& operator=(const Variable&);
		Variable& operator=(Variable&&) noexcept;

		Variable root() const;
		Variable subpath(DataPath&& path) const;

	private:
		void increment() const;
		void decrement() const;

	public:
		const DataPoint& ref() const;
		DataPoint& ref();
		DataPoint consume() &&;

		template<typename T>
		T consume_as() &&;

		const DataPath* path() const;
		DataPath* path();

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
