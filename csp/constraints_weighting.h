#pragma once

#include "pch.h"
#include "constraint_problem.h"

namespace csp
{
	template <typename T>
	const AssignmentHistory<T> constraintWeighting(ConstraintProblem<T>& constraintProblem, unsigned int maxTries,
		bool writeAssignmentHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		std::optional<AssignmentHistory<T>> optAssignmentHistory = std::optional<AssignmentHistory<T>>{ assignmentHistory };
		const std::vector<std::reference_wrapper<Variable<T>>>& vecReadOnlyVars = constraintProblem.getAssignedVariables();
		std::unordered_set<std::reference_wrapper<Variable<T>>> readOnlyVars{ vecReadOnlyVars.cbegin(), vecReadOnlyVars.cend() };
		std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>> optReadOnlyVars{ readOnlyVars };
		const std::vector<std::reference_wrapper<Constraint<T>>>& constraints = constraintProblem.getConstraints();
		std::map<std::reference_wrapper<Constraint<T>>, int> constraintToWeightMap;
		for (Constraint<T>& constr : constraints)
		{
			constraintToWeightMap.emplace(constr, 1);
		}

		for (unsigned int i = 0; i < maxTries; ++i)
		{
			constraintProblem.assignVarsWithRandomValues(optReadOnlyVars, optAssignmentHistory);
			int lastReduction = std::numeric_limits<int>::max();
			while (0 < lastReduction)
			{
				if (constraintProblem.isCompletelyConsistentlyAssigned())
				{
					return assignmentHistory;
				}

				auto [reduction, varRef, value] = __geBestReductionVariableValue<T>(constraintProblem, readOnlyVars, constraintToWeightMap);
				Variable<T>& var = varRef.get();
				var.unassign();
				if (writeAssignmentHistory)
				{
					assignmentHistory.emplace_back(var, std::optional<T>{});
				}
				var.assign(value);
				if (writeAssignmentHistory)
				{
					assignmentHistory.emplace_back(var, std::optional<T>{value});
				}

				lastReduction = reduction;

				for (Constraint<T>& unsatisfiedConstraint : constraintProblem.getUnsatisfiedConstraints())
				{
					++constraintToWeightMap[unsatisfiedConstraint];
				}
			}

			if (i != maxTries - 1)
			{
				constraintProblem.unassignAllVariables();
			}
		}

		return assignmentHistory;
	}
	
	template <typename T>
	static std::tuple<int, std::reference_wrapper<Variable<T>>, T> __geBestReductionVariableValue(ConstraintProblem<T>& constraintProblem, 
		const std::unordered_set<std::reference_wrapper<Variable<T>>>& readOnlyVars,
		std::map<std::reference_wrapper<Constraint<T>>, int>& constraintToWeightMap)
	{
		int weight = __CalculateWeight<T>(constraintProblem, constraintToWeightMap);
		const Assignment<T>& originalAssignment = constraintProblem.getCurrentAssignment();
		constraintProblem.unassignAllVariables();
		std::map<int, VariableValuePair<T>> weightToPairMap;
		for (Variable<T>& var : constraintProblem.getVariables())
		{
			if (readOnlyVars.count(var))
			{
				continue;
			}
			for (T value : var.getDomain())
			{
				var.assign(value);
				int currWeight = __CalculateWeight<T>(constraintProblem, constraintToWeightMap);
				weightToPairMap.emplace(weight - currWeight, VariableValuePair<T>{var, value});
				var.unassign();
			}
		}

		constraintProblem.unassignAllVariables();
		constraintProblem.assignFromAssignment(originalAssignment);
		const auto& maxReduction = weightToPairMap.rbegin();
		// TODO: write it in a more readable way
		return { maxReduction->first, maxReduction->second.first, maxReduction->second.second };
	}

	template <typename T>
	static int __CalculateWeight(ConstraintProblem<T>& constraintProblem, 
		std::map<std::reference_wrapper<Constraint<T>>, int>& constraintToWeightMap)
	{
		int weight = 0;
		for (Variable<T>& var : constraintProblem.getVariables())
		{
			const std::vector<std::reference_wrapper<Constraint<T>>>& constraintsContainingVar = 
				constraintProblem.getConstraintsContainingVariable(var);
			std::vector<std::reference_wrapper<Constraint<T>>> unsatisfiedConstraintsContainingVar;
			unsatisfiedConstraintsContainingVar.reserve(constraintsContainingVar.size());
			for (Constraint<T>& constr : constraintsContainingVar)
			{
				if (!constr.isSatisfied())
				{
					unsatisfiedConstraintsContainingVar.emplace_back(constr);
				}
			}

			for (Constraint<T>& constr : unsatisfiedConstraintsContainingVar)
			{
				weight += constraintToWeightMap[constr];
			}
		}
		return weight;
	}
}