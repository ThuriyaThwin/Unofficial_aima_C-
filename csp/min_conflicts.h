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
	static size_t __getMinConflictedAssignmentIdx(ConstraintProblem<T>& constraintProblem, Variable<T>& conflictedVar)
	{
		size_t minConflictsCount = std::numeric_limits<size_t>::max();
		const std::vector<T>& conflictedVarDomain = conflictedVar.getDomain();

		std::vector<size_t> minConflictingAssignmentIdxs;
		for (size_t i = 0; i < conflictedVarDomain.size(); ++i)
		{
			conflictedVar.assignByIdx(i);
			size_t conflictsCount = constraintProblem.getUnsatisfiedConstraintsSize();
			if (conflictsCount < minConflictsCount)
			{
				minConflictsCount = conflictsCount;
				minConflictingAssignmentIdxs.clear();
				minConflictingAssignmentIdxs.push_back(i);
			}
			else if (conflictsCount == minConflictsCount)
			{
				minConflictingAssignmentIdxs.push_back(i);
			}
			conflictedVar.unassign();

		}

		return __selectElementRandomly<size_t, std::vector<size_t>>(minConflictingAssignmentIdxs);
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
			
			size_t minConflictedAssignmentIdx = __getMinConflictedAssignmentIdx<T>(constraintProblem, conflictedVar);
			conflictedVar.assignByIdx(minConflictedAssignmentIdx);
			if (writeAssignmentHistory)
			{
				const std::vector<T>& domain = conflictedVar.getDomain();
				assignmentHistory.emplace_back(conflictedVar, std::optional<T>{ domain[minConflictedAssignmentIdx] });
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