#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	using PrimarySelector = std::function<std::vector<std::reference_wrapper<Variable<T>>>(ConstraintProblem<T>& constraintProblem)>;
	template <typename T>
	using SecondarySelector = std::function<Variable<T>& (ConstraintProblem<T>&, const std::vector<std::reference_wrapper<Variable<T>>>& candidateVars)>;
	template <typename T>
	using DomainSorter = std::function<const std::vector<T>(ConstraintProblem<T>&, Variable<T>&)>;
	template <typename T>
	using Inference = std::function<bool(ConstraintProblem<T>&, Variable<T>& assignedVariable)>;

	template <typename T>
	const AssignmentHistory<T> heuristicBacktrackingSolver(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector = chooseFirstCandidateVar_secondarySelector<T>,
		const std::optional<DomainSorter<T>>& optDomainSorter = std::optional<DomainSorter<T>>{},
		const std::optional<Inference<T>> optInference = std::optional<Inference<T>>{},
		bool writeAssignmentHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		__heuristicBacktrackingSolver(constraintProblem, primarySelector, secondarySelector, optDomainSorter, optInference, 
			writeAssignmentHistory, assignmentHistory);
		return assignmentHistory;
	}

	template <typename T>
	static bool __heuristicBacktrackingSolver(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector,
		const std::optional<DomainSorter<T>>& optDomainSorter,
		const std::optional<Inference<T>>& optInference,
		bool writeAssignmentHistory,
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

		const std::vector<std::reference_wrapper<Variable<T>>>& candidateVars = primarySelector(constraintProblem);
		Variable<T>& selectedVar = candidateVars.size() == 1 ? candidateVars[0].get() : secondarySelector(constraintProblem, candidateVars);

		const std::vector<T>& sortedDomain = optDomainSorter ? (*optDomainSorter)(constraintProblem, selectedVar) : selectedVar.getDomain();
		for (T value : sortedDomain)
		{
			selectedVar.assign(value);
			if (writeAssignmentHistory)
			{
				assignmentHistory.emplace_back(selectedVar, std::optional<T>{value});
			}

			if (optInference && !((*optInference)(constraintProblem, selectedVar)))
			{
				selectedVar.unassign();
				if (writeAssignmentHistory)
				{
					assignmentHistory.emplace_back(selectedVar, std::optional<T>{});
				}
				return false;
			}

			if (__heuristicBacktrackingSolver(constraintProblem, primarySelector, secondarySelector, optDomainSorter, optInference,
				writeAssignmentHistory, assignmentHistory))
			{
				return true;
			}

			selectedVar.unassign();
			if (writeAssignmentHistory)
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
		const std::optional<DomainSorter<T>>& optDomainSorter = std::optional<DomainSorter<T>>{},
		const std::optional<Inference<T>> optInference = std::optional<Inference<T>>{})
	{
		std::unordered_set<Assignment<T>> solutions;
		__heuristicBacktrackingSolver_findAllSolutions(constraintProblem, primarySelector, secondarySelector, domainSorter,
			optInference, solutions);
		return solutions;
	}

	template <typename T>
	static void __heuristicBacktrackingSolver_findAllSolutions(ConstraintProblem<T>& constraintProblem,
		const PrimarySelector<T>& primarySelector,
		const SecondarySelector<T>& secondarySelector,
		const std::optional<DomainSorter<T>>& optDomainSorter,
		const std::optional<Inference<T>>& optInference,
		std::unordered_set<Assignment<T>>& solutions)
	{
		if (constraintProblem.isCompletelyAssigned())
		{
			if (constraintProblem.isConsistentlyAssigned())
			{
				solutions.emplace(constraintProblem.getCurrentAssignment());
			}
			return;
		}

		const std::vector<std::reference_wrapper<Variable<T>>>& candidateVars = primarySelector(constraintProblem);
		Variable<T>& selectedVar = candidateVars.size() == 1 ? candidateVars[0].get() : secondarySelector(constraintProblem, candidateVars);

		const std::vector<T>& sortedDomain = optDomainSorter ? (*optDomainSorter)(constraintProblem, selectedVar) : selectedVar.getDomain();
		for (T value : sortedDomain)
		{
			selectedVar.assign(value);

			if (optInference && (*optInference)(constraintProblem, selectedVar))
			{
				selectedVar.unassign();
				return;
			}

			__heuristicBacktrackingSolver_findAllSolutions(constraintProblem, primarySelector, secondarySelector, optDomainSorter,
				optInference, solutions);

			selectedVar.unassign();
		}
	}
}