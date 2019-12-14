#pragma once

#include "pch.h"
#include "constraint_problem.h"


template <typename T>
using VariableValueNeighborTriplet = std::tuple<std::reference_wrapper<csp::Variable<T>>, T, std::reference_wrapper<csp::Variable<T>>>;


namespace std
{
	template <typename T>
	struct std::hash<typename csp::VariableValuePair<T>>
	{
		size_t operator()(const csp::VariableValuePair<T>& variableValuePair) const
		{
			std::hash<T> valueHasher;
			std::hash<std::reference_wrapper<csp::Variable<T>>> varRefHasher;
			double hashValue = varRefHasher(get<0>(variableValuePair));
			hashValue /= valueHasher(get<1>(variableValuePair));
			return static_cast<size_t>(hashValue);
		}
	};
}

namespace std
{
	template <typename T>
	struct std::hash<typename VariableValueNeighborTriplet<T>>
	{
		size_t operator()(const VariableValueNeighborTriplet<T>& variableValueNeighborTriplet) const
		{
			std::hash<T> valueHasher;
			std::hash<std::reference_wrapper<csp::Variable<T>>> varRefHasher;
			double hashValue = varRefHasher(get<0>(variableValueNeighborTriplet));
			hashValue /= valueHasher(get<1>(variableValueNeighborTriplet));
			hashValue /= varRefHasher(get<2>(variableValueNeighborTriplet));
			return static_cast<size_t>(hashValue);
		}
	};
}

namespace csp
{
	template <typename T>
	bool ac4(ConstraintProblem<T>& constraintProblem)
	{
		std::unordered_map<VariableValueNeighborTriplet<T>, int> supportCounter;
		std::unordered_map<VariableValuePair<T>, std::unordered_set<VariableValuePair<T>>> variableValuePairsSupportedBy;
		std::queue<VariableValuePair<T>> unsupportedVariableValuePairs;
		__initialize_ac4(constraintProblem.getConstraints(), supportCounter, variableValuePairsSupportedBy, unsupportedVariableValuePairs);

		while (!unsupportedVariableValuePairs.empty())
		{
			const VariableValuePair<T>& secondVarValuePair = unsupportedVariableValuePairs.front();
			unsupportedVariableValuePairs.pop();
			for (const VariableValuePair<T>& firstVarValuePair : variableValuePairsSupportedBy[secondVarValuePair])
			{
				T firstValue = firstVarValuePair.second;
				Variable<T>& firstVar = firstVarValuePair.first;
				const std::vector<T>& firstDomain = firstVar.getDomain();
				const auto& firstDomainItEnd = firstDomain.cend();
				const auto& firstDomainItBegin = firstDomain.cbegin();
				auto serachRes = std::find(firstDomainItBegin, firstDomainItEnd, firstValue);
				if (serachRes != firstDomainItEnd)
				{
					VariableValueNeighborTriplet<T> variableValueNeighborTriplet = 
						std::make_tuple(std::ref(firstVar), firstValue, std::ref(secondVarValuePair.first));
					--supportCounter[variableValueNeighborTriplet];
					if (!supportCounter[variableValueNeighborTriplet])
					{
						firstVar.removeFromDomain(std::distance(firstDomainItBegin, serachRes));
						unsupportedVariableValuePairs.emplace(firstVarValuePair);
					}
				}
			}
		}

		return constraintProblem.isPotentiallySolvable();
	}

	template <typename T>
	static void __initialize_ac4(const std::vector<std::reference_wrapper<Constraint<T>>>& constraints,
		std::unordered_map<VariableValueNeighborTriplet<T>, int>& supportCounter,
		std::unordered_map<VariableValuePair<T>, std::unordered_set<VariableValuePair<T>>>& variableValuePairsSupportedBy,
		std::queue<VariableValuePair<T>>& unsupportedVariableValuePairs)
	{
		for (const Constraint<T>& constr : constraints)
		{
			const std::vector<std::reference_wrapper<Variable<T>>>& variables = constr.getVariables();
			if (variables.size() == 1)
			{
				continue;
			}
			Variable<T>* firstVar = &(variables[0].get());
			Variable<T>* secondVar = &(variables[1].get());

			for (int i = 0; i < 2; ++i)
			{
				bool firstVarWasAssigned = firstVar->isAssigned();
				const std::vector<T>& firstDomain = firstVar->getDomain();
				for (size_t i = 0; i < firstDomain.size(); ++i)
				{
					T firstValue = firstDomain[i];
					if (!firstVarWasAssigned)
					{
						firstVar->assign(firstValue);
					}
					bool secondVarWasAssigned = secondVar->isAssigned();
					VariableValuePair<T> firstPair = std::make_pair(std::ref(*firstVar), firstValue);
					VariableValueNeighborTriplet<T> variableValueNeighborTriplet = 
						std::make_tuple(std::ref(*firstVar), firstValue, std::ref(*secondVar));
					for (T secondValue : secondVar->getDomain())
					{
						if (!secondVarWasAssigned)
						{
							secondVar->assign(secondValue);
						}
						if (constr.isConsistent())
						{
							supportCounter.try_emplace(variableValueNeighborTriplet, 0);
							++supportCounter[variableValueNeighborTriplet];
							VariableValuePair<T> secondPair = std::make_pair(std::ref(*secondVar), secondValue);
							variableValuePairsSupportedBy.try_emplace(secondPair, std::unordered_set<VariableValuePair<T>>{});
							variableValuePairsSupportedBy[secondPair].emplace(firstPair);
						}
						if (!secondVarWasAssigned)
						{
							secondVar->unassign();
						}
					}
					if (!firstVarWasAssigned)
					{
						firstVar->unassign();
					}
					if (!supportCounter[variableValueNeighborTriplet])
					{
						firstVar->removeFromDomain(i);
						unsupportedVariableValuePairs.emplace(firstPair);
					}
				}
				std::swap(firstVar, secondVar);
			}
		}
	}
}