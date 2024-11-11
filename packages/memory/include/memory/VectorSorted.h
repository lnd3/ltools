#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include "logging/Log.h"
#include "meta/Reflection.h"

namespace l::container {

	template<class T>
	class VectorSorted {
	public:

		//-------------------------------------------------------------------//
		// SetSorted()																			//
		//-------------------------------------------------------------------//
		// I didn't want to override every constructor to set this
		// member variable, so this function is publicly accessible.
		// You should call SetSorted( true ) right after construction.
		// TO DO: if you feel like it... derive all constructors to avoid
		// the need for this.  There are 4 last time I checked.
		//-------------------------------------------------------------------//
		void SetSorted(bool bSorted = true) { m_bSorted = bSorted; }

		//-------------------------------------------------------------------//
		// sort()                                                            //
		//-------------------------------------------------------------------//
		// This function sorts the data as needed.  Call it after repeated calls to
		// push_unsorted(), or just let other members call it for you on next access.
		// It calls std::sort(), which defaults to using operator<() for
		// comparisons.
		//-------------------------------------------------------------------//
		void sort() {
			if (!m_bSorted)
			{
				std::sort(vec.begin(), vec.end());
				SetSorted();
			}
		}

		typename std::vector<T>::iterator lower_bound_it(const T& key) {
			if (!m_bSorted)
				sort();

			typename std::vector<T>::iterator it = std::lower_bound(vec.begin(), vec.end(), key);
			return it;
		}

		/*const*/ T* lower_bound_ptr(const T& key) {
			typename std::vector<T>::iterator it = lower_bound_it(key);

			if (it == vec.end())
				return 0;

			/*const*/ T* t = &(*it);
			return t;
		}


		//-------------------------------------------------------------------//
		// find_it_or_fail()                                                 //
		//-------------------------------------------------------------------//
		// This function takes the given object and determines if there is
		// a match in the vector.  It returns an iterator to the actual
		// object in the vector, if found.  Otherwise returns std::vector::end().
		//
		// This is the function you want to use most of the time
		// (or the predicate version if you are using object pointers).
		//
		// USAGE: it makes most sense to use this function if you have
		// an object with a key, other member variables, and operator<()
		// that uses the key to test for equality.  You then set up a dummy
		// "search" object with the key set to the search value, call the
		// function, and use the result to extract the additional information
		// from the object.
		//-------------------------------------------------------------------//
		typename std::vector<T>::iterator find_it_or_fail(const T& key) {
			typename std::vector<T>::iterator it = lower_bound_it(key);

			if (it != vec.end())

				// lower_bound() does not necessarily indicate a successful search.
				// The iterator points to the object where an insertion
				// should take place.  We check that result to see if we actually
				// had an exact match.

				// NOTE: This is how the STL determines equality using only operator()<.
				// Two comparisons, ugg, but it is a nice little trick.
				if (!((*it) < key) && !(key < (*it)))

					return it;

			return vec.end();
		}

		//-------------------------------------------------------------------//
		// find_ptr_or_fail()                                                 //
		//-------------------------------------------------------------------//
		// A variation of find_it_or_fail() that provides a pointer to result.
		//-------------------------------------------------------------------//
		T* find_ptr_or_fail(const T& key) {
			typename std::vector<T>::iterator it = find_it_or_fail(key);
			if (it != vec.end())
				return &(*it);

			return 0;
		}

		//-------------------------------------------------------------------//
		// push_sorted()																		//
		//-------------------------------------------------------------------//
		// This is used to insert into a vector that always remains sorted.
		// Because we have a sorted vector, finding the insertion location
		// with std::lower_bound() is relatively cheap.
		//
		// If you have multiple insertions, consider
		// using push_unsorted() for each, then calling sort().
		//-------------------------------------------------------------------//
		void push_sorted(const T& t) {
			if (!m_bSorted)
			{
				sort();
			}

			// Insert at "lower_bound" (the proper sorted location).
			vec.insert(std::lower_bound(vec.begin(), vec.end(), t), t);
		}

		//-------------------------------------------------------------------//
		// push_unsorted()																	//
		//-------------------------------------------------------------------//
		// This is similar to push_back(), but in addition, it sets the
		// unsorted flag.
		//-------------------------------------------------------------------//
		void push_unsorted(const T& t) {
			SetSorted(false);
			vec.push_back(t);
		}

		//-------------------------------------------------------------------//
		// operator=()																	//
		//-------------------------------------------------------------------//
		// This allows us to set the VectorSorted from a std::vector.
		//-------------------------------------------------------------------//
		VectorSorted<T>& operator=(std::vector<T>& v) {
			typename std::vector<T>::iterator it;
			for (it = v.begin(); it != v.end(); ++it)
				push_unsorted((*it));
			return this;
		}

		// CALLS WHERE YOU PROVIDE THE FUNCTOR OR FUNCTION POINTER
		// If you need to use a predicate sort function, ALWAYS use these methods
		// instead of the non-functor versions.
		// NOTE: UPDATE THIS when C++0x polymorphic function wrappers are available.
		template<class _Pr>
		inline void sort(_Pr pr) {
			if (!m_bSorted)
			{
				std::sort(vec.begin(), vec.end(), pr);
				SetSorted();
			}
		}
		template<class _Pr>
		inline typename std::vector<T>::iterator lower_bound_it(const T& key, _Pr pr) {
			if (!m_bSorted)
			{
				std::sort(vec.begin(), vec.end(), pr);
				SetSorted();
			}
			typename std::vector<T>::iterator it = std::lower_bound(vec.begin(), vec.end(), key, pr);
			return it;
		}

		template<class _Pr>
		inline /*const*/ T* lower_bound_ptr(const T& key, _Pr pr) {
			typename std::vector<T>::iterator it = lower_bound_it(key, pr);

			if (it == vec.end())
				return 0;

			/*const*/ T* t = &(*it);
			return t;
		}

		template<class _Pr> 
		inline void push_sorted(const T& t, _Pr pr) {
			if (!m_bSorted)
			{
				std::sort(vec.begin(), vec.end(), pr);
				SetSorted();
			}

			// Insert at "lower_bound" (the proper sorted location).
			vec.insert(std::lower_bound(vec.begin(), vec.end(), t, pr), t);
		}

		template<class _Pr>
		inline typename std::vector<T>::iterator find_it_or_fail(const T& key, _Pr pr) {
			typename std::vector<T>::iterator it = lower_bound_it(key, pr);

			if (it != vec.end())

				// NOTE: We have to apply this using the predicate function, be careful...
				if (!(pr((*it), key)) && !(pr(key, (*it))))

					return it;

			return vec.end();
		}

		template<class _Pr> 
		inline T* find_ptr_or_fail(const T& key, _Pr pr) {
			typename std::vector<T>::iterator it = find_it_or_fail(key, pr);
			if (it != vec.end())
				return &(*it);

			return 0;
		}

		inline typename std::vector<T>::iterator begin() noexcept {
			return vec.begin();
		}

		inline typename std::vector<T>::iterator end() noexcept {
			return vec.end();
		}

		inline size_t size() const noexcept {
			return vec.size();
		}

		inline bool empty() const noexcept {
			return vec.empty();
		}

	protected:
		std::vector<T> vec;
		bool m_bSorted;
	};

}
