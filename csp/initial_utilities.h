#pragma once

#include "pch.h"

// CSPDOs in general:
// switch syntax of all "constructor(container.cbegin(), container.cend())" to constructor{ container.cbegin(), container.cend() }"
/* figure out a way to know in compile time whether we should use "for (T val : values)" or "for (const T& val : values).
   same goes for functions returning T by-value instead of by reference, like Variabke.getValue() for example. std::is_fundamental might be useful. */
// allow users to determine it they want the whole assignment history or simply the number of assignments and un-assignments.
// rewrite various solvers so that they'll output their assignment history.
/* in variable's constructor allow users to give a way to keep the domain vector sorted, so that searches could be made using binary search.
   maybe use SFINAE to check if std::less<T> is compilable. If you do this then you can use std::includes in setSubsetDomain */
// use make_optional wherever you make a std::optional

// CSPDOs for c++20:
// use coroutines in backtracking
// use wait and notify on atomics in ConstraintProblem<T>::isCompletelyConsistentlyAssigned()
// use a concept for T in Variable<T>
// make this library a module


namespace csp
{
	template <typename U>
	using Ref = std::reference_wrapper<U>;
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