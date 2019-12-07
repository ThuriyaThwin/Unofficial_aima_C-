#pragma once

#include "pch.h"
#include "arc_consistency_3.h"


namespace csp
{
	template <typename T>
	bool mac(ConstraintProblem<T>& constraintProblem, Variable<T>& assignedVariable)
	{
		std::unordered_set<VariableRefsPair<T>>& arcs = __initArcsMAC(constraintProblem, assignedVariable);
		return ac3(constraintProblem, arcs);
	}

	template <typename T>
	static std::unordered_set<VariableRefsPair<T>> __initArcsMAC(ConstraintProblem<T>& constraintProblem, Variable<T>& assignedVariable)
	{
		std::unordered_set<VariableRefsPair<T>> arcs;
		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedNeighbors = constraintProblem.getUnassignedNeighbors(assignedVariable);
		arcs.reserve(unassignedNeighbors.size());
		for (Variable<T>& unassignedNeigh : unassignedNeighbors)
		{
			arcs.emplace(unassignedNeigh, assignedVariable);
		}
		return arcs;
	}
}