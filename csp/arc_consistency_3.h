#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	using VariableRefsPair = std::pair<Ref<Variable<T>>, Ref<Variable<T>>>;
}

namespace std
{
	template <typename T>
	struct std::hash<typename csp::VariableRefsPair<T>>
	{
		size_t operator()(const csp::VariableRefsPair<T>& variableRefsPair) const
		{
			std::hash<std::reference_wrapper<csp::Variable<T>>> varHasher;
			double firstVarHashValue = static_cast<double>(varHasher(variableRefsPair.first));
			size_t secondVarHashValue = varHasher(variableRefsPair.second);
			double hashKey = firstVarHashValue / secondVarHashValue;
			return static_cast<size_t>(hashKey);
		}
	};
}


namespace csp
{
	template <typename T>
	static void __eraseUnassignedNeighborFromNeighbors(std::vector<Ref<Variable<T>>>& neighbors, Variable<T>& unassignedNeighbor)
	{
		for (auto it = neighbors.cbegin(); it != neighbors.cend(); ++it)
		{
			if ((*it).get() == unassignedNeighbor)
			{
				neighbors.erase(it);
				return;
			}
		}
	}

	template <typename T>
	static std::optional<Constraint<T>> __findSharedConstraint(const std::vector<Ref<Constraint<T>>>& firstVariableConstraints,
		const std::vector<Ref<Constraint<T>>>& secondVariableConstraints)
	{
		std::optional<Constraint<T>> sharedConstr;
		size_t firstSize = firstVariableConstraints.size();
		size_t secondSize = secondVariableConstraints.size();
		const std::vector<Ref<Constraint<T>>>& shorterVec = firstSize <= secondSize ? firstVariableConstraints : secondVariableConstraints;
		const std::vector<Ref<Constraint<T>>>& longerVec = firstSize <= secondSize ? secondVariableConstraints : firstVariableConstraints;
		bool notFoundSharedConstraint = true;

		for (auto it = shorterVec.begin(); it != shorterVec.end() && notFoundSharedConstraint; ++it)
		{
			for (Constraint<T>& constrOfLonger : longerVec)
			{
				if (*it == constrOfLonger)
				{
					sharedConstr = (*it).get();
					notFoundSharedConstraint = false;
					break;
				}
			}
		}
		return sharedConstr;
	}

	template <typename T>
	static bool __revise(ConstraintProblem<T>& constraintProblem, Variable<T>& variable, Variable<T>& neighbor)
	{
		if (variable.isAssigned())
		{
			return false;
		}
		bool revised = false;
		const std::vector<Ref<Constraint<T>>>& variableConstraints = constraintProblem.getConstraintsContainingVariable(variable);
		const std::vector<Ref<Constraint<T>>>& neighborConstraints = constraintProblem.getConstraintsContainingVariable(neighbor);
		std::optional<Constraint<T>> optSharedConstr = __findSharedConstraint<T>(variableConstraints, neighborConstraints);
		if (optSharedConstr)
		{
			Constraint<T>& sharedConstraint = *optSharedConstr;
			const std::vector<T>& variableDomain = variable.getDomain();
			for (size_t i = 0; i < variableDomain.size(); ++i)
			{
				variable.assignByIdx(i);
				//variable.assign(variableDomain[i]);
				if (sharedConstraint.getConsistentDomainValues(neighbor).empty())
				{
					variable.unassign();
					variable.removeFromDomain(i);
					revised = true;
				}
				variable.unassign();
			}
		}
		return revised;
	}
}


namespace csp
{
	template <typename T>
	bool ac3(ConstraintProblem<T>& constraintProblem, std::unordered_set<VariableRefsPair<T>>& arcs)
	{
		while (!arcs.empty())
		{
			const auto& itToStart = arcs.cbegin();
			auto [unassignedVar, unassignedNeighbor] = *itToStart;
			arcs.erase(itToStart);
			if (__revise<T>(constraintProblem, unassignedVar.get(), unassignedNeighbor.get()))
			{
				if (constraintProblem.getConsistentDomain(unassignedVar).empty())
				{
					return false;
				}

				std::vector<Ref<Variable<T>>> neighbors =
					const_cast<std::vector<Ref<Variable<T>>>&>(constraintProblem.getNeighbors(unassignedVar));
				__eraseUnassignedNeighborFromNeighbors<T>(neighbors, unassignedNeighbor.get());

				if (!neighbors.empty())
				{
					for (Variable<T>& neigh : neighbors)
					{
						arcs.emplace(neigh, unassignedVar);
					}
				}
			}
		}

		return constraintProblem.isPotentiallySolvable();
	}

	template <typename T>
	std::unordered_set<VariableRefsPair<T>> initArcsAC3(ConstraintProblem<T>& constraintProblem)
	{
		std::unordered_set<VariableRefsPair<T>> arcs;
		const std::vector<Ref<Variable<T>>> unassignedVars = constraintProblem.getUnassignedVariables();
		arcs.reserve(unassignedVars.size());
		for (Variable<T>& unassignedVar : unassignedVars)
		{
			for (Variable<T>& unassignedNeighbor : constraintProblem.getNeighbors(unassignedVar))
			{
				arcs.emplace(unassignedVar, unassignedNeighbor);
			}
		}
		return arcs;
	}
}