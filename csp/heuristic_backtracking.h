#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	using PrimarySelector = std::function<std::vector<std::reference_wrapper<Variable<T>>>(ConstraintProblem<T>& constraintProblem)>;
	template <typename T>
	using SecondarySelector = std::function<Variable<T> & (ConstraintProblem<T>&, const std::vector<std::reference_wrapper<Variable<T>>> & candidateVars)>;
	template <typename T>
	using DomainSorter = std::function<const std::vector<T>(ConstraintProblem<T>&, Variable<T>&)>;
	template <typename T>
	using Inference = std::function<bool(ConstraintProblem<T>&, Variable<T> & assignedVariable)>;

	template <typename T>
	const AssignmentHistory<T> heuristicBacktrackingSolver(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector = csp::chooseFirstCandidateVar_secondarySelector,
		const std::optional<DomainSorter<T>>& domainSorter = std::optional<DomainSorter<T>>{},
		const std::optional<Inference<T>> inference = std::optional<Inference<T>>{},
		bool withHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		__heuristicBacktrackingSolver(constraintProblem, primarySelector, secondarySelector, domainSorter, inference, withHistory,
			assignmentHistory);
		return assignmentHistory;
	}

	template <typename T>
	static bool __heuristicBacktrackingSolver(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector,
		const std::optional<DomainSorter<T>>& domainSorter,
		const std::optional<Inference<T>>& inference,
		bool withHistory,
		AssignmentHistory<T>& assignmentHistory)
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

		const std::vector<std::reference_wrapper<Variable<T>>> candidateVars = primarySelector(constraintProblem);
		Variable<T>& selectedVar = candidateVars.size() == 1 ? candidateVars[0].get() : secondarySelector(constraintProblem, candidateVars);

		const std::vector<T>& sortedDomain = domainSorter ? domainSorter.value()(constraintProblem, selectedVar) : selectedVar.getDomain();
		for (const T value : sortedDomain)
		{
			selectedVar.assign(value);
			if (withHistory)
			{
				assignmentHistory.emplace_back(selectedVar, std::optional<T>{value});
			}

			if (inference && inference.value()(constraintProblem, selectedVar))
			{
				selectedVar.unassign();
				if (withHistory)
				{
					assignmentHistory.emplace_back(selectedVar, std::optional<T>{});
				}
				return false;
			}

			if (__heuristicBacktrackingSolver(constraintProblem, primarySelector, secondarySelector, domainSorter, inference,
				withHistory, assignmentHistory))
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
	const std::unordered_set<Assignment<T>> heuristicBacktrackingSolver_findAllSolutions(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector = csp::chooseFirstCandidateVar_secondarySelector,
		const std::optional<DomainSorter<T>>& domainSorter = std::optional<DomainSorter<T>>{},
		const std::optional<Inference<T>> inference = std::optional<Inference<T>>{})
	{
		std::unordered_set<Assignment<T>> solutions;
		__heuristicBacktrackingSolver_findAllSolutions(constraintProblem, primarySelector, secondarySelector, domainSorter,
			inference, solutions);
		return solutions;
	}

	template <typename T>
	static void __heuristicBacktrackingSolver_findAllSolutions(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector,
		const std::optional<DomainSorter<T>>& domainSorter,
		const std::optional<Inference<T>>& inference,
		std::unordered_set<Assignment<T>>& solutions)
	{
		if (constraintProblem.isCompletelyAssigned())
		{
			if (constraintProblem.isConsistentlyAssigned())
			{
				const Assignment<T>& sol = constraintProblem.getCurrentAssignment();
				solutions.insert(sol);
			}
			return;
		}

		const std::vector<std::reference_wrapper<Variable<T>>> candidateVars = primarySelector(constraintProblem);
		Variable<T>& selectedVar = candidateVars.size() == 1 ? candidateVars[0].get() : secondarySelector(constraintProblem, candidateVars);

		const std::vector<T>& sortedDomain = domainSorter ? domainSorter.value()(constraintProblem, selectedVar) : selectedVar.getDomain();
		for (const T value : sortedDomain)
		{
			selectedVar.assign(value);

			if (inference && inference.value()(constraintProblem, selectedVar))
			{
				selectedVar.unassign();
				return;
			}

			__heuristicBacktrackingSolver_findAllSolutions(constraintProblem, primarySelector, secondarySelector, domainSorter,
				inference, solutions);

			selectedVar.unassign();
		}
	}
}