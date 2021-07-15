#pragma once

//https://stackoverflow.com/questions/36517825/is-stephen-lavavejs-mallocator-the-same-in-c11/36521845#36521845

#include <stdlib.h> // size_t, malloc, free
#include <new> // bad_alloc, bad_array_new_length

template <class T> struct Mallocator {
	using value_type = T;
	
	Mallocator() = default;
	
	template <class U> Mallocator(const Mallocator<U>&) noexcept { }
	
	template <class U> bool operator==(
		const Mallocator<U>&) const noexcept {
		return true;
	}
	template <class U> bool operator!=(
		const Mallocator<U>&) const noexcept {
		return false;
	}

	T* allocate(const size_t n) const {
		if (n == 0) { return nullptr; }
		if (n > static_cast<size_t>(-1) / sizeof(T)) {
			throw std::bad_array_new_length();
		}
		void* const pv = malloc(n * sizeof(T));
		if (!pv) { throw std::bad_alloc(); }
		return static_cast<T*>(pv);
	}
	void deallocate(T* const p, size_t) const noexcept {
		free(p);
	}
};