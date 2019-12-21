#pragma once

#include "constraint_problem.h"


namespace csp
{
	template <typename T>	/* Good score is high score */
	using ScoreCalculator = std::function<unsigned int(ConstraintProblem<T>&)>;

	template <typename T>
	unsigned int consistentConstraintsAmount(ConstraintProblem<T>& constraintProblem)
	{
		return static_cast<unsigned int>(constraintProblem.getConsistentConstraintsSize());
	}
}