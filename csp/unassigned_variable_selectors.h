#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	Variable<T>& chooseFirstCandidateVar_secondarySelector(ConstraintProblem<T>& constraintProblem,
		const std::vector<Ref<Variable<T>>>& candidateVariables)
	{
		return candidateVariables.front().get();
	}

	template <typename T>
	const std::vector<Ref<Variable<T>>> minimumRemainingValues_primarySelector(ConstraintProblem<T>& constraintProblem)
	{
		const std::vector<Ref<Variable<T>>> unassignedVariables = constraintProblem.getUnassignedVariables();
		std::multimap<size_t, Ref<Variable<T>>> scoreToVarMap;
		for (Variable<T>& unassignedVar : unassignedVariables)
		{
			const std::vector<T> consistentDomain = constraintProblem.getConsistentDomain(unassignedVar);
			scoreToVarMap.emplace(consistentDomain.size() , unassignedVar);
		}

		size_t smallestConsistentDomainSize = scoreToVarMap.cbegin()->first;
		std::vector<Ref<Variable<T>>> variables;
		variables.reserve(scoreToVarMap.size() >> 2);
		for (const std::pair<size_t, Ref<Variable<T>>>& scoreToVar : scoreToVarMap)
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
		const std::vector<Ref<Variable<T>>>& candidateVariables)
	{
		std::multimap<size_t, Ref<Variable<T>>> scoreToVarMap;
		for (Variable<T>& var : candidateVariables)
		{
			const std::vector<T> consistentDomain = constraintProblem.getConsistentDomain(var);
			scoreToVarMap.emplace(consistentDomain.size() , var);
		}
		return scoreToVarMap.cbegin()->second;
	}

	template <typename T>
	const std::vector<Ref<Variable<T>>> degreeHeuristic_primarySelector(const ConstraintProblem<T>& constraintProblem)
	{
		const std::vector<Ref<Variable<T>>> unassignedVariables = constraintProblem.getUnassignedVariables();
		std::multimap<size_t, Ref<Variable<T>>> scoreToVarMap;
		for (Variable<T>& unassignedVar : unassignedVariables)
		{
			const std::vector<Ref<Variable<T>>> unassignedNeighbors = constraintProblem.getUnassignedNeighbors(unassignedVar);
			scoreToVarMap.emplace(unassignedNeighbors.size() , unassignedVar);
		}

		size_t biggestUnassignedNeighborsSize = scoreToVarMap.crbegin()->first;
		std::vector<Ref<Variable<T>>> variables;
		variables.reserve(scoreToVarMap.size() >> 2);
		for (const auto it = scoreToVarMap.crbegin(); it != scoreToVarMap.crend(); ++it)
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
		const std::vector<Ref<Variable<T>>>& candidateVariables)
	{
		std::multimap<size_t, Ref<Variable<T>>> scoreToVarMap;
		for (Variable<T>& var : candidateVariables)
		{
			const std::vector<Ref<Variable<T>>> unassignedNeighbors = constraintProblem.getUnassignedNeighbors(var);
			scoreToVarMap.emplace(unassignedNeighbors.size() , var );
		}
		return scoreToVarMap.crbegin()->second;
	}
}