#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	Variable<T>& chooseFirstCandidateVar_secondarySelector(ConstraintProblem<T>& constraintProblem,
		const std::vector<std::reference_wrapper<Variable<T>>>& candidateVariables)
	{
		return candidateVariables[0].get();
	}

	template <typename T>
	const std::vector<std::reference_wrapper<Variable<T>>> minimumRemainingValues_primarySelector(ConstraintProblem<T>& constraintProblem)
	{
		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedVariables = constraintProblem.getUnassignedVariables();
		std::multimap<int, std::reference_wrapper<Variable<T>>> scoreToVarMap;
		for (Variable<T>& unassignedVar : unassignedVariables)
		{
			const std::vector<T>& consistentDomain = constraintProblem.getConsistentDomain(unassignedVar);
			scoreToVarMap.insert({ consistentDomain.size() , unassignedVar });
		}

		int smallestConsistentDomainSize = scoreToVarMap.cbegin()->first;
		std::vector<std::reference_wrapper<Variable<T>>> variables;
		variables.reserve(scoreToVarMap.size() >> 2);
		for (const std::pair<int, std::reference_wrapper<Variable<T>>>& scoreToVar : scoreToVarMap)
		{
			if (scoreToVar.first == smallestConsistentDomainSize)
			{
				variables.emplace_back(scoreToVar.second);
			}
			else
			{
				break;
			}
		}
		return variables;
	}

	template <typename T>
	Variable<T>& minimumRemainingValues_secondarySelector(ConstraintProblem<T>& constraintProblem,
		const std::vector<std::reference_wrapper<Variable<T>>>& candidateVariables)
	{
		std::multimap<int, std::reference_wrapper<Variable<T>>> scoreToVarMap;
		for (Variable<T>& var : candidateVariables)
		{
			const std::vector<T>& consistentDomain = constraintProblem.getConsistentDomain(var);
			scoreToVarMap.insert({ consistentDomain.size() , var });
		}
		return scoreToVarMap.cbegin()->second;
	}

	template <typename T>
	const std::vector<std::reference_wrapper<Variable<T>>> degreeHeuristic_primarySelector(const ConstraintProblem<T>& constraintProblem)
	{
		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedVariables = constraintProblem.getUnassignedVariables();
		std::multimap<int, std::reference_wrapper<Variable<T>>> scoreToVarMap;
		for (Variable<T>& unassignedVar : unassignedVariables)
		{
			const std::vector<std::reference_wrapper<Variable<T>>>& unassignedNeighbors = constraintProblem.getUnassignedNeighbors(unassignedVar);
			scoreToVarMap.insert({ unassignedNeighbors.size() , unassignedVar });
		}

		int biggestUnassignedNeighborsSize = scoreToVarMap.crbegin()->first;
		std::vector<std::reference_wrapper<Variable<T>>> variables;
		variables.reserve(scoreToVarMap.size() >> 2);
		for (const auto& it = scoreToVarMap.crbegin(); it != scoreToVarMap.crend(); ++it)
		{
			if (it->first == biggestUnassignedNeighborsSize)
			{
				variables.emplace_back(it->second);
			}
			else
			{
				break;
			}
		}
		return variables;
	}

	template <typename T>
	Variable<T>& degreeHeuristic_secondarySelector(const ConstraintProblem<T>& constraintProblem,
		const std::vector<std::reference_wrapper<Variable<T>>>& candidateVariables)
	{
		std::multimap<int, std::reference_wrapper<Variable<T>>> scoreToVarMap;
		for (Variable<T>& var : candidateVariables)
		{
			const std::vector<std::reference_wrapper<Variable<T>>>& unassignedNeighbors = constraintProblem.getUnassignedNeighbors(var);
			scoreToVarMap.insert({ unassignedNeighbors.size() , var });
		}
		return scoreToVarMap.crbegin()->second;
	}
}