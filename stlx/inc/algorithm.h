#ifndef __STLX_ALGORITHM_H__
#define __STLX_ALGORITHM_H__

#include <exception>
#include <algorithm>

namespace stdx
{
	class not_found_exception : public std::exception
	{
	public:
		not_found_exception()
			:exception("item not found")
		{
		}
	};

	
	template <typename ContainerT>
	struct container_value_type
	{
		typedef typename ContainerT::value_type value_type;
	};

	template <typename T, int N>
	struct container_value_type<T[N]>
	{
		typedef T value_type;
	};

	template <typename T>
	struct container_value_type<T*>
	{
		typedef T value_type;
	};

	template <typename Container1T, typename Container2T>
	void assert_contains_same_type()
	{
		static_assert(std::is_same<typename container_value_type<Container1T>::value_type, typename container_value_type<Container2T>::value_type>::value, "container types don't match");
	}

	template <typename T>
	struct ptr_container_wrapper
	{
		typedef T value_type;

		value_type * begin_;
		value_type * end_;

		ptr_container_wrapper(T *begin, T *end)
			:begin_(begin), end_(end)
		{
		}

		T* begin()
		{
			return begin_;
		}

		T* end()
		{
			return end_;
		}

		const T* begin() const
		{
			return begin_;
		}

		const T* end() const
		{
			return end_;
		}

		
	};

	/////////////////////////////////////////////////////////////////////////////////
	/// ContainerT - ContainerConcept (supports begin && end)
	template <typename ContainerT>
	typename container_value_type<ContainerT>::value_type find(const ContainerT &container, const typename container_value_type<ContainerT>::value_type &value)
	{
		auto i = std::find(std::begin(container), std::end(container), value);
		if (i == std::end(container))
			throw not_found_exception();
		return *i;
	}
		
	template <typename ContainerT, typename PredicateT>
	typename container_value_type<ContainerT>::value_type& find_if(ContainerT &container, PredicateT &&pred)
	{
		auto i = std::find_if(std::begin(container), std::end(container),std::forward<PredicateT>(pred));
		if (i == std::end(container))
			throw not_found_exception();
		return (*i);
	}

	template <typename ContainerT, typename PredicateT>
	const typename container_value_type<ContainerT>::value_type& find_if(const ContainerT &container, PredicateT &&pred)
	{
		return find_if(container,std::forward<PredicateT>(pred));
	}

	template <typename ContainerT, typename PredicateT>
	bool find_if(ContainerT &container, typename container_value_type<ContainerT>::value_type &value, PredicateT &&pred)
	{
		auto i = std::find_if(std::begin(container), std::end(container), std::forward<PredicateT>(pred));
		if (i == std::end(container))
			return false;
		value = *i;
		return true;
	}

	template <typename ContainerT, typename PredicateT>
	bool find_if(const ContainerT &container, typename container_value_type<ContainerT>::value_type &value, PredicateT &&pred)
	{
		return find_if(container,value,std::forward<PredicateT>(pred))
	}
	
	template <typename ContainerT>
	bool exists(const ContainerT &container, const typename container_value_type<ContainerT>::value_type &value)
	{
		auto i = std::find(std::begin(container), std::end(container), value);
		return i != std::end(container);
	}

	template <typename ContainerT>
	bool erase(ContainerT &container, const typename container_value_type<ContainerT>::value_type &value)
	{
		auto i = std::remove(std::begin(container), std::end(container), value);
		if (i == std::end(container))
			return false;
		container.erase(i, std::end(container));
		return true;
	}

	template <typename ContainerT, typename PredicateT>
	bool erase_if(ContainerT &container, const PredicateT &pred)
	{
		auto i = std::remove_if(std::begin(container), std::end(container), pred);
		if (i == std::end(container))
			return false;
		container.erase(i, std::end(container));
		return true;
	}

	template <typename ContainerT>
	bool insert_before(ContainerT &container, const typename container_value_type<ContainerT>::value_type &value, const typename container_value_type<ContainerT>::value_type &insert_v)
	{
		auto i = std::find(std::begin(container), std::end(container), value);
		if (i == std::end(container))
			return false;
		container.insert(i, insert_v);
		return true;
	}

	template <typename ContainerT, typename PredicateT>
	bool insert_before(ContainerT &container, const PredicateT &pred, const typename container_value_type<ContainerT>::value_type &value)
	{
		auto i = std::find_if(std::begin(container), std::end(container), pred);
		if (i == std::end(container))
			return false;
		container.insert(i, value);
	}

	template <typename ContainerT>
	bool insert_after(ContainerT &container, const typename container_value_type<ContainerT>::value_type &value, const typename container_value_type<ContainerT>::value_type &insert_v)
	{
		auto i = std::find(std::begin(container), std::end(container), value);
		if (i == std::end(container))
			return false;
		container.insert(i+1, insert_v);
		return true;
	}

	template <typename ContainerT, typename PredicateT>
	bool insert_after(ContainerT &container, const PredicateT &pred, const typename container_value_type<ContainerT>::value_type &value)
	{
		auto i = std::find_if(std::begin(container), std::end(container), pred);
		if (i == std::end(container))
			return false;

		container.insert(i+1, value);
		return true;
	}

	template <typename Container1T, typename Container2T>
	bool contains(const Container1T &c1, const Container2T &c2)
	{
		assert_contains_same_type<Container1T, Container2T>();

		auto b = std::begin(c2), b_end(std::end(c2)); // store the start of the search.
		auto i = std::begin(c1), i_end(std::end(c1));

		// fast check to ensure it's possible.
		if (std::distance(b, b_end) > std::distance(i, i_end))
			return false;

		while (i != i_end)
		{
			if (*i++ == *b++)
			{
				if (b == b_end)
					return true;
			}
			else
				b = std::begin(c2);
		}
		return false;
	}

	template <typename Container1T, typename Container2T>
	bool equals(const Container1T &c1, const Container2T &c2)
	{
		assert_contains_same_type<Container1T, Container2T>();

		auto l1 = std::distance(std::begin(c1), std::end(c1));
		auto l2 = std::distance(std::begin(c2), std::end(c2));

		if (l1 != l2)
			return false;

		auto i = std::begin(c1), i_end(std::end(c1));
		auto b = std::begin(c2), b_end(std::end(c2));

		while (i != i_end)
		{
			if (*i++ != *b++)
				return false;
		}
		return true;
	}

	template <typename Container1T, typename Container2T>
	bool starts_with()
	{

	}

	

}

#endif // __STLX_ALGORITHM_H__