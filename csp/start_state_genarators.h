#pragma once

#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	using StartStateGenarator = std::function<ConstraintProblem<T>(ConstraintProblem<T>&,
		std::vector<Variable<T>>&, std::vector<Constraint<T>>&)>;

	template <typename T>
	ConstraintProblem<T> generateStartStateRandomly(ConstraintProblem<T>& srcConstraintProblem,
		std::vector<Variable<T>>& destVars, std::vector<Constraint<T>>& destConstraints)
	{
		ConstraintProblem<std::string>& copiedConstraintProb = srcConstraintProblem.deepCopy(destVars, destConstraints);
		copiedConstraintProb.unassignAllVariables();
		copiedConstraintProb.assignVarsWithRandomValues();
		return std::move(copiedConstraintProb);
	}
}