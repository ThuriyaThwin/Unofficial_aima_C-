#pragma once

#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	using SuccessorGenerator = std::function<ConstraintProblem<T>(ConstraintProblem<T>&,
		std::vector<Variable<T>>&, std::vector<Constraint<T>>&)>;

	template <typename T>
	ConstraintProblem<T> alterRandomVariableValuePair(ConstraintProblem<T>& srcConstraintProblem,
		std::vector<Variable<T>>& destVars, std::vector<Constraint<T>>& destConstraints)
	{
		ConstraintProblem<std::string>& copiedConstraintProb = srcConstraintProblem.deepCopy(destVars, destConstraints);
		Variable<T>& randomlySelectedVar = __selectElementRandomly<std::reference_wrapper<Variable<T>>,
			std::vector<std::reference_wrapper<Variable<T>>>>(copiedConstraintProb.getVariables());
		std::optional<T> optOldValue;
		if (randomlySelectedVar.isAssigned())
		{
			optOldValue = randomlySelectedVar.getValue();
		}
		const std::vector<T>& varDomain = randomlySelectedVar.getDomain();
		T newValue = __selectElementRandomly<T, std::vector<T>>(varDomain);
		while (optOldValue && newValue == *optOldValue && 1 < varDomain.size())
		{
			newValue = __selectElementRandomly<T, std::vector<T>>(varDomain);
		}
		randomlySelectedVar.unassign();
		randomlySelectedVar.assign(newValue);
		return std::move(copiedConstraintProb);
	}
}