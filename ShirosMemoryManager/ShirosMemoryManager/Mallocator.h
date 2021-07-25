#pragma once

//https://stackoverflow.com/questions/36517825/is-stephen-lavavejs-mallocator-the-same-in-c11/36521845#36521845

#include <stdlib.h> // size_t, malloc, free
#include <new> // bad_alloc, bad_array_new_length

template <class T> 
class Mallocator {
public:
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using value_type = T;

	explicit Mallocator() = default;

	/*Rebind a Mallocator<T> to a Mallocator<U> */
	template <class U> 
	struct rebind { typedef Mallocator<U> other; };
	
	template <class U> 
	Mallocator(const Mallocator<U>&) {}

	inline pointer address(reference ref) const { return &ref; }
	inline const_pointer address(const_reference ref) const { return &ref; }
	inline size_type max_size() const throw() { return std::numeric_limits<size_t>::max() / sizeof(value_type); }

	inline pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
	{
		if (n == 0) { return nullptr; }
		if (n > std::numeric_limits<size_t>::max() / sizeof(T)) {
			throw std::bad_array_new_length();
		}
		void* const pv = malloc(n * sizeof(T));
		if (!pv) { throw std::bad_alloc(); }
		return static_cast<pointer>(pv);
	}

	inline void deallocate(pointer p, size_type n)
	{
		free(p);
	}

	inline void construct(pointer p, const T & val)
	{
		new(static_cast<void*>(p)) T(val); //object construction only
	}

	inline void construct(pointer p)
	{
		new(static_cast<void*>(p)) T();  //object construction only
	}

	inline void destroy(pointer p)
	{
		p->~T(); //object destruction only
	}

	inline bool operator==(const Mallocator& a) { return &a == this; }
	inline bool operator!=(const Mallocator& a) { return !operator==(a); }

};