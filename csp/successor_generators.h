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
		ConstraintProblem<T> copiedConstraintProb = srcConstraintProblem.deepCopy(destVars, destConstraints);
		Variable<T>& randomlySelectedVar = __selectElementRandomly<Ref<Variable<T>>,
			std::vector<Ref<Variable<T>>>>(copiedConstraintProb.getVariables());
		std::optional<T> optOldValue;
		if (randomlySelectedVar.isAssigned())
		{
			optOldValue = randomlySelectedVar.getValue();
		}

		size_t domainSize = randomlySelectedVar.getDomain().size();
		do 
		{
			randomlySelectedVar.unassign();
			randomlySelectedVar.assignWithRandomlySelectedValue();
		} 
		while (1 < domainSize && optOldValue && *optOldValue == randomlySelectedVar.getValue());

		return copiedConstraintProb;
	}
}