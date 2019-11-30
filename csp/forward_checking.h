#pragma once

#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	bool forwardChecking(ConstraintProblem<T>& constraintProblem, Variable<T>& assignedVariable)
	{
		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedNeighbors = constraintProblem.getUnassignedNeighbors(assignedVariable);
		for (Variable<T>& unassignedNeighbor : unassignedNeighbors)
		{
			const std::vector<T>& consistentDomain = constraintProblem.getConsistentDomain(unassignedNeighbor);
			if (!consistentDomain.size())
			{
				return true;
			}
		}
		return false;
	}
}