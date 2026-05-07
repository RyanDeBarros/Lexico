#pragma once

#include <memory>

namespace lx
{
	class DataPoint;
	class DataHeap;

	class Variable
	{
		DataHeap* _heap;
		unsigned int _id;
		bool _temporary;
		std::shared_ptr<unsigned int> _ref_count;

	public:
		Variable(DataHeap& heap, unsigned int id, bool temporary);
		Variable(const Variable&);
		Variable(Variable&&) noexcept;
		~Variable();
		Variable& operator=(const Variable&);
		Variable& operator=(Variable&&) noexcept;

	private:
		void decrement();

	public:
		const DataPoint& ref() const;
		DataPoint& ref();
		DataPoint dp();

		bool temporary() const;

		size_t hash() const;
		bool operator==(const Variable& other) const = default;
	};
}

template<>
struct std::hash<lx::Variable>
{
	size_t operator()(const lx::Variable& var) const;
};
