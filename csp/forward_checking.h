#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	bool forwardChecking(ConstraintProblem<T>& constraintProblem, Variable<T>& assignedVariable)
	{
		const std::vector<Ref<Variable<T>>>& unassignedNeighbors = constraintProblem.getUnassignedNeighbors(assignedVariable);
		for (Variable<T>& unassignedNeighbor : unassignedNeighbors)
		{
			const std::vector<T>& consistentDomain = constraintProblem.getConsistentDomain(unassignedNeighbor);
			if (consistentDomain.empty())
			{
				return false;
			}
		}
		return true;
	}
}