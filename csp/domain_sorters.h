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
		std::multimap<int, T> scoreToValueMap;
		const std::vector<std::reference_wrapper<Variable<T>>>& unassignedNeighbors = constraintProb.getUnassignedNeighbors(var);
		const std::vector<T>& consistentDomain = constraintProb.getConsistentDomain(var);
		for (T value : consistentDomain)
		{
			var.assign(value);
			int neighborDomainLengthsSum = 0;
			for (const auto& unassignedNeighbor : unassignedNeighbors)
			{
				const std::vector<T>& neighborConsistentDomain = constraintProb.getConsistentDomain(unassignedNeighbor);
				neighborDomainLengthsSum += neighborConsistentDomain.size();
			}
			scoreToValueMap.insert({ neighborDomainLengthsSum, value });
			var.unassign();
		}

		std::vector<T> sortedDomain;
		sortedDomain.reserve(scoreToValueMap.size());
		std::transform(scoreToValueMap.cbegin(), scoreToValueMap.cend(), back_inserter(sortedDomain),
			[](const std::pair<int, T>& scoreToValue) -> T { return scoreToValue.second; });
		return sortedDomain;
	}
}