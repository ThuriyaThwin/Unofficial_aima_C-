#pragma once

#include "pch.h"

// CSPDOs in general:
// use std::is_fundamental and std::conditional to ascertain whether to pass T around by value or by const reference
// allow users to determine it they want the whole assignment history or simply the number of assignments and un-assignments.
// rewrite various solvers so that they'll output their assignment history.


// CSPDOs for c++20:
// use coroutines in backtracking
// use wait and notify on atomics in ConstraintProblem<T>::isCompletelyConsistentlyAssigned()
// use a concept for T in Variable<T>
// turn this library into a module


namespace csp
{
	template <typename U>
	using Ref = std::reference_wrapper<U>;

	static constexpr size_t UNASSIGNED = std::numeric_limits<size_t>::max();
}

namespace csp
{
	template<typename S, typename T, typename = void>
	struct __is_to_stream_writable : std::false_type
	{ };

	template<typename Stream, typename T>
	struct __is_to_stream_writable<Stream, T, std::void_t<decltype(std::declval<Stream&>() << std::declval<T>())>> : std::true_type
	{ };

	template <typename T>
	bool __compare_T(const T& left, const T& right)
	{
		/* CSPDO: if T is implicitly convertible to boolean,
		then the compiler would convert left and right to booleans and perform
		a function call that would returns a boolean, hence returning a boolean from this function
		as well, which means that this so-called test has failed because this is not what we intended. */
		return left < right;
	}

	template <typename T>
	using __T_less_than_operator_return_type = typename std::result_of<decltype(__compare_T<T>)&(const T& left, const T& right)>::type;
}


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