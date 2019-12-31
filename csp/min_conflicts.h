#pragma once

#include "pch.h"
#include "constraint_problem.h"


template<typename T>
class invalid_tabu_size_error : public std::invalid_argument
{
public:
	invalid_tabu_size_error() :
		invalid_argument{ "Given tabu_size is bigger than the number of variables in the constraint problem or\n"
			"(tabu_size + read_only_variables_size) is bigger than the number of variables in the constraint problem." }
	{ }
};

namespace csp
{
	template <typename T>
	static Variable<T>& __getRandomConflictedVariable(ConstraintProblem<T>& constraintProblem,
		std::optional<std::unordered_set<Ref<Variable<T>>>> optReadOnlyVars)
	{
		const std::vector<Ref<Variable<T>>>& variables = constraintProblem.getVariables();
		std::unordered_set<Ref<Variable<T>>> conflictedVars;
		conflictedVars.reserve(variables.size());
		for (const Constraint<T>& constr : constraintProblem.getUnsatisfiedConstraints())
		{
			const std::vector<Ref<Variable<T>>>& constrVars = constr.getVariables();
			conflictedVars.insert(constrVars.cbegin(), constrVars.cend());
		}
		if (optReadOnlyVars)
		{
			for (auto it = conflictedVars.begin(); it != conflictedVars.end(); )
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
			__selectElementRandomly<Ref<Variable<T>>, std::unordered_set<Ref<Variable<T>>>>(conflictedVars);
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

	template <typename T>
	const AssignmentHistory<T> minConflicts(ConstraintProblem<T>& constraintProblem, unsigned int maxSteps,
		std::optional<std::unordered_set<Ref<Variable<T>>>> optReadOnlyVars =
		std::optional<std::unordered_set<Ref<Variable<T>>>>{},
		size_t tabuSize = 0, 
		bool writeAssignmentHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		if (optReadOnlyVars && constraintProblem.getVariables().size() <= tabuSize + (*optReadOnlyVars).size() ||
			constraintProblem.getVariables().size() <= tabuSize)
		{
			// CSPDO: test it
			throw invalid_tabu_size_error<T>{};
		}
		
		constraintProblem.assignVarsWithRandomValues(optReadOnlyVars, assignmentHistory);

		size_t bestMinConflicts = constraintProblem.getUnsatisfiedConstraintsSize();
		Assignment<T> bestAssignment = const_cast<Assignment<T>&>(constraintProblem.getCurrentAssignment());
		Assignment<T>* ptrBestAssignment = &(bestAssignment);
		for (unsigned int i = 0; i < maxSteps; ++i)
		{
			if (constraintProblem.isCompletelyConsistentlyAssigned())
			{
				return assignmentHistory;
			}

			Variable<T>& conflictedVar = __getRandomConflictedVariable<T>(constraintProblem, optReadOnlyVars);
			conflictedVar.unassign();
			if (writeAssignmentHistory)
			{
				assignmentHistory.emplace_back(conflictedVar, std::optional<T>{});
			}
			
			T minConflictedValue = __getMinConflictedValue<T>(constraintProblem, conflictedVar);
			conflictedVar.assign(minConflictedValue);
			if (writeAssignmentHistory)
			{
				assignmentHistory.emplace_back(conflictedVar, std::optional<T>{ minConflictedValue });
			}

			size_t currConflictsCount = constraintProblem.getUnsatisfiedConstraintsSize();
			if (currConflictsCount < bestMinConflicts)
			{
				bestMinConflicts = currConflictsCount;
				const Assignment<T> currAssignment = constraintProblem.getCurrentAssignment();
				*ptrBestAssignment = const_cast<Assignment<T>&>(currAssignment);
			}
		}

		constraintProblem.unassignAllVariables();
		constraintProblem.assignFromAssignment(*ptrBestAssignment);
		return assignmentHistory;
	}
}