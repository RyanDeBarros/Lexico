#pragma once

#include <memory>

namespace lx
{
	class DataPoint;
	class VirtualHeap;

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
		DataPoint dp();

		bool unbound() const;

		size_t hash() const;
		bool operator==(const Variable& other) const = default;
	};
}

template<>
struct std::hash<lx::Variable>
{
	size_t operator()(const lx::Variable& var) const;
};
