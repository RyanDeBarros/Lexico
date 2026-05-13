#pragma once

#include <memory>
#include <utility>

namespace lx
{
	template<typename T>
	class CowPtr
	{
		std::shared_ptr<T> _ptr;
		std::shared_ptr<size_t> _ref_count;

	public:
		explicit CowPtr() requires std::default_initializable<T>
			: _ptr(std::make_shared<T>()), _ref_count(std::make_shared<size_t>(1))
		{
		}

		CowPtr(const CowPtr<T>& other)
			: _ptr(other._ptr), _ref_count(other._ref_count)
		{
			if (_ref_count)
				++*_ref_count;
		}

		CowPtr(CowPtr<T>&& other)
			: _ptr(std::move(other._ptr)), _ref_count(std::move(other._ref_count))
		{
		}

		~CowPtr()
		{
			if (_ref_count)
				--*_ref_count;
		}

		CowPtr& operator=(const CowPtr<T>& other)
		{
			if (this != &other)
			{
				if (_ptr != other._ptr)
				{
					if (_ref_count)
						--*_ref_count;
					_ptr = other._ptr;
					_ref_count = other._ref_count;
					if (_ref_count)
						++*_ref_count;
				}
			}
			return *this;
		}

		CowPtr& operator=(CowPtr<T>&& other)
		{
			if (this != &other)
			{
				if (_ptr != other._ptr)
				{
					if (_ref_count)
						--*_ref_count;
					_ptr = std::move(other._ptr);
					_ref_count = std::move(other._ref_count);
				}
			}
			return *this;
		}

		CowPtr(const T& value)
			: _ptr(std::make_shared<T>(value)), _ref_count(std::make_shared<size_t>(1))
		{
		}

		CowPtr(T&& value)
			: _ptr(std::make_shared<T>(std::move(value))), _ref_count(std::make_shared<size_t>(1))
		{
		}

		template<typename... Args>
		CowPtr(std::in_place_t, Args&&... args)
			: _ptr(std::make_shared<T>(std::forward<Args>(args)...)), _ref_count(std::make_shared<size_t>(1))
		{
		}

		const T& operator*() const
		{
			return *_ptr;
		}

		T& operator*()
		{
			detach();
			return *_ptr;
		}

		const T* operator->() const
		{
			return _ptr;
		}

		T* operator->()
		{
			detach();
			return _ptr;
		}

		bool operator==(const CowPtr<T>& other) const
		{
			return this == &other || _ptr == other._ptr || *_ptr == *other._ptr;
		}

	private:
		void detach()
		{
			if (_ref_count && *_ref_count > 1)
			{
				--*_ref_count;
				_ptr = std::make_shared<T>(*_ptr);
				_ref_count = std::make_shared<size_t>(1);
			}
		}
	};

	namespace detail
	{
		template<typename T>
		struct remove_cow
		{
			using type = T;
			static constexpr bool value = false;
		};

		template<typename T>
		struct remove_cow<CowPtr<T>>
		{
			using type = T;
			static constexpr bool value = true;
		};
	}

	template<typename T>
	using remove_cow_t = detail::remove_cow<std::decay_t<T>>::type;

	template<typename T>
	constexpr bool is_cow_v = detail::remove_cow<std::decay_t<T>>::value;

	template<typename T>
	decltype(auto) remove_cow(T&& obj)
	{
		if constexpr (is_cow_v<T>)
		{
			if constexpr (std::is_rvalue_reference_v<T&&>)
				return std::move(*obj);
			else
				return *obj;
		}
		else
			return std::forward<T>(obj);
	}
}
