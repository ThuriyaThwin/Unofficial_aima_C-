#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	const AssignmentHistory<T> minConflicts(ConstraintProblem<T>& constraintProblem, unsigned int maxSteps,
		std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>> optReadOnlyVars =
		std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>>{},
		size_t tabuSize = 0, 
		bool writeAssignmentHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		if (optReadOnlyVars && constraintProblem.getVariables().size() <= tabuSize + (*optReadOnlyVars).size() ||
			constraintProblem.getVariables().size() <= tabuSize)
		{
			// TODO: write an exception
			throw 1;
		}
		
		constraintProblem.assignVarsWithRandomValues(optReadOnlyVars, assignmentHistory);

		size_t bestMinConflicts = constraintProblem.getUnsatisfiedConstraintsSize();
		const Assignment<T>* bestMinConflictsAssignment = &(constraintProblem.getCurrentAssignment());
		for (unsigned int i = 0; i < maxSteps; ++i)
		{
			if (constraintProblem.isCompletelyConsistentlyAssigned())
			{
				return assignmentHistory;	
			}

			Variable<T>& conflictedVar = __getRandomConflictedVariable(constraintProblem, optReadOnlyVars);
			conflictedVar.unassign();
			if (writeAssignmentHistory)
			{
				assignmentHistory.emplace_back(conflictedVar, std::optional<T>{});
			}
			
			T minConflictedValue = __getMinConflictedValue<T>(constraintProblem, conflictedVar);
			conflictedVar.assign(minConflictedValue);
			if (writeAssignmentHistory)
			{
				assignmentHistory.emplace_back(conflictedVar, std::optional<T>{minConflictedValue});
			}

			size_t currConflictsCount = constraintProblem.getUnsatisfiedConstraintsSize();
			if (currConflictsCount < bestMinConflicts)
			{
				bestMinConflicts = currConflictsCount;
				bestMinConflictsAssignment = &(constraintProblem.getCurrentAssignment());
			}
		}

		constraintProblem.unassignAllVariables();
		constraintProblem.assignFromAssignment(*bestMinConflictsAssignment);
		return assignmentHistory;
	}

	template <typename T>
	static Variable<T>& __getRandomConflictedVariable(ConstraintProblem<T>& constraintProblem,
		std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>> optReadOnlyVars)
	{
		const std::vector<std::reference_wrapper<Variable<T>>>& variables = constraintProblem.getVariables();
		std::unordered_set<std::reference_wrapper<Variable<T>>> conflictedVars;
		conflictedVars.reserve(variables.size());
		for (const Constraint<T>& constr : constraintProblem.getUnsatisfiedConstraints())
		{
			const std::vector<std::reference_wrapper<Variable<T>>>& constrVars = constr.getVariables();
			conflictedVars.insert(constrVars.cbegin(), constrVars.cend());
		}
		if (optReadOnlyVars)
		{
			for (auto& it = conflictedVars.begin(); it != conflictedVars.end(); )
			{
				if ((*optReadOnlyVars).count(*it))
				{
					it = conflictedVars.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		Variable<T>& var = 
			__selectElementRandomly<std::reference_wrapper<Variable<T>>, std::unordered_set<std::reference_wrapper<Variable<T>>>>(conflictedVars);
		return var;
	}

	template <typename T>
	static T __getMinConflictedValue(ConstraintProblem<T>& constraintProblem, Variable<T>& conflictedVar)
	{
		size_t minConflictsCount = std::numeric_limits<size_t>::max();
		const std::vector<T>& conflictedVarDomain = conflictedVar.getDomain();
		std::vector<T> minConflictingValues;
		minConflictingValues.reserve(conflictedVarDomain.size());
		for (T value : conflictedVarDomain)
		{
			conflictedVar.assign(value);
			size_t conflictsCount = constraintProblem.getUnsatisfiedConstraintsSize();
			if (conflictsCount < minConflictsCount)
			{
				minConflictsCount = conflictsCount;
				minConflictingValues.clear();
				minConflictingValues.emplace_back(value);
			}
			else if (conflictsCount == minConflictsCount)
			{
				minConflictingValues.emplace_back(value);
			}
			conflictedVar.unassign();
		}

		return __selectElementRandomly<T, std::vector<T>>(minConflictingValues);
	}
}