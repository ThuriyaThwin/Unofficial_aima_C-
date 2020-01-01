#pragma once

#include "pch.h"

// CSPDOs in general:
// switch syntax of all "constructor(container.cbegin(), container.cend())" to constructor{ container.cbegin(), container.cend() }"
// use std::is_fundamental and std::conditional to ascertain whether to pass T around by value or by const reference
// allow users to determine it they want the whole assignment history or simply the number of assignments and un-assignments.
// rewrite various solvers so that they'll output their assignment history.
/* in variable's constructor allow users to give a way to keep the domain vector sorted, so that searches could be made using binary search.
   maybe use SFINAE to check if std::less<T> is compilable. If you do this then you can use std::includes in setSubsetDomain */
// use make_optional wherever you make a std::optional

// CSPDOs for c++20:
// use coroutines in backtracking
// use wait and notify on atomics in ConstraintProblem<T>::isCompletelyConsistentlyAssigned()
// use a concept for T in Variable<T>
// turn this library into a module


namespace csp
{
	template <typename U>
	using Ref = std::reference_wrapper<U>;

	template <typename T>
	using optCompare = std::optional<std::function<constexpr bool(T left, T right)>>;
}

namespace csp
{
	template<typename S, typename T, typename = void>
	struct __is_to_stream_writable : std::false_type
	{ };

	template<typename Stream, typename T>
	struct __is_to_stream_writable<Stream, T, std::void_t<decltype(std::declval<Stream&>() << std::declval<T>())>> : std::true_type
	{ };

	template<class, class = std::void_t<>>
	struct __is_comparable_to_T : std::false_type { };

	template<class T>
	struct __is_comparable_to_T< T, std::void_t< decltype(std::declval<const T&>() < std::declval<const T&>())> > : std::true_type { };

	template <typename T>
	bool __compare_T(const T& left, const T& right)
	{
		return left < right;
	}

	template <typename T>
	using __T_less_than_operator_return_value = typename std::result_of<decltype(__compare_T<T>)&(const T& left, const T& right)>::type;

}


// CSPDO: perhaps use std::result_of https://en.cppreference.com/w/cpp/types/result_of
// http://www.cplusplus.com/reference/type_traits/result_of/



namespace csp
{
	template <typename T, typename Container>
	T __selectElementRandomly(const Container& containerIn)
	{
		Container containerOut;
		std::sample(containerIn.cbegin(), containerIn.cend(), std::inserter(containerOut, containerOut.end()), 1,
			std::default_random_engine{ std::random_device{}() });
		return *(containerOut.cbegin());
	}
}