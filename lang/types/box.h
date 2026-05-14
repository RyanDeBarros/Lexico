#pragma once

#include <memory>

namespace lx
{
	template<typename T>
	class Box
	{
		std::unique_ptr<T> _ptr;

	public:
		Box(const T& val) : _ptr(std::make_unique<T>(val)) {}
		Box(T&& val) : _ptr(std::make_unique<T>(std::move(val))) {}

		T& operator*() { return *_ptr; }
		const T& operator*() const { return *_ptr; }
		T* operator->() { return _ptr; }
		const T* operator->() const { return _ptr; }
	};
}
