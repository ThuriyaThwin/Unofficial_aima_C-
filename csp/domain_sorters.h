#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	const std::vector<T> doNotSort(const ConstraintProblem<T>& constraintProblem, const Variable<T>& variable)
	{
		return constraintProblem.getConsistentDomain(variable);
	}

	template <typename T>
	const std::vector<T> leastConstrainingValue(ConstraintProblem<T>& constraintProb, Variable<T>& var)
	{
		std::multimap<size_t, T> scoreToValueMap;
		const std::vector<Ref<Variable<T>>> unassignedNeighbors = constraintProb.getUnassignedNeighbors(var);
		const std::vector<T> consistentDomain = constraintProb.getConsistentDomain(var);
		for (T value : consistentDomain)
		{
			var.assignByValue(value);
			size_t neighborDomainLengthsSum = 0;
			for (Variable<T>& unassignedNeigh : unassignedNeighbors)
			{
				const std::vector<T> neighborConsistentDomain = constraintProb.getConsistentDomain(unassignedNeigh);
				neighborDomainLengthsSum += neighborConsistentDomain.size();
			}
			scoreToValueMap.emplace(neighborDomainLengthsSum, value);
			var.unassign();
		}

		std::vector<T> sortedDomain;
		sortedDomain.reserve(scoreToValueMap.size());
		std::transform(scoreToValueMap.cbegin(), scoreToValueMap.cend(), back_inserter(sortedDomain),
			[](const std::pair<size_t, T>& scoreToValue) -> T { return scoreToValue.second; });
		return sortedDomain;
	}
}