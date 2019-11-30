#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	const AssignmentHistory<T> backtrackingSolver(const ConstraintProblem<T>& constraintProblem, bool withHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		__backtrackingSolver(constraintProblem, assignmentHistory, withHistory);
		return assignmentHistory;
	}

	template <typename T>
	static bool __backtrackingSolver(const ConstraintProblem<T>& constraintProblem, AssignmentHistory<T>& assignmentHistory, bool withHistory)
	{
		if (constraintProblem.isCompletelyAssigned())
		{
			if (constraintProblem.isConsistentlyAssigned())
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedVars = constraintProblem.getUnassignedVariables();
		Variable<T>& selectedVar = unassignedVars[0];
		for (T value : selectedVar.getDomain())
		{
			selectedVar.assign(value);
			if (withHistory)
			{
				assignmentHistory.emplace_back(selectedVar, std::optional<T>{value});
			}

			if (__backtrackingSolver(constraintProblem, assignmentHistory, withHistory))
			{
				return true;
			}

			selectedVar.unassign();
			if (withHistory)
			{
				assignmentHistory.emplace_back(selectedVar, std::optional<T>{});
			}
		}
		return false;
	}

	template <typename T>
	const std::unordered_set<Assignment<T>> backtrackingSolver_findAllSolutions(const ConstraintProblem<T>& constraintProblem)
	{
		unordered_set<Assignment<T>> solutions;
		__backtrackingSolver_findAllSolutions(constraintProblem, solutions);
		return solutions;
	}

	template <typename T>
	void __backtrackingSolver_findAllSolutions(const ConstraintProblem<T>& constraintProblem, std::unordered_set<Assignment<T>>& solutions)
	{
		if (constraintProblem.isCompletelyAssigned())
		{
			if (constraintProblem.isConsistentlyAssigned())
			{
				solutions.emplace(constraintProblem.getCurrentAssignment());
			}
			return;
		}

		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedVars = constraintProblem.getUnassignedVariables();
		Variable<T>& selectedVar = unassignedVars[0];
		for (const T value : selectedVar.getDomain())
		{
			selectedVar.assign(value);
			__backtrackingSolver_findAllSolutions(constraintProblem, solutions);
			selectedVar.unassign();
		}
	}
}