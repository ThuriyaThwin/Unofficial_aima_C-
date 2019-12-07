#pragma once

#include "pch.h"
#include "constraint_problem.h"


template <typename T>
using VarDiffVarNeighborTriplet = std::tuple<std::reference_wrapper<csp::Variable<T>>, std::reference_wrapper<csp::Variable<T>>, 
	std::reference_wrapper<csp::Variable<T>>>;


namespace csp
{
	template <typename T>
	bool pc2(ConstraintProblem<T>& constraintProblem)
	{
		std::queue<VarDiffVarNeighborTriplet<T>> varDiffVarNeighborTriplets;
		const std::vector<std::reference_wrapper<Variable<T>>>& variables = constraintProblem.getVariables();
		for (Variable<T>& var : variables)
		{
			for (Variable<T>& neighbor : constraintProblem.getNeighbors(var))
			{
				std::vector<std::reference_wrapper<Variable<T>>> diffVariables{ variables };
				__eraseVarAndNeighborFromDiffVariables<T>(diffVariables, var, neighbor);
				for (Variable<T>& diffVar : diffVariables)
				{
					VarDiffVarNeighborTriplet<T> varDiffVarNeighborTri = std::make_tuple(std::ref(var), std::ref(neighbor), std::ref(diffVar));
					varDiffVarNeighborTriplets.emplace(varDiffVarNeighborTri);
				}
			}
		}

		while (!varDiffVarNeighborTriplets.empty())
		{
			const auto& [var, diffVar, neighbor] = varDiffVarNeighborTriplets.front();
			varDiffVarNeighborTriplets.pop();
			if (__revise3<T>(constraintProblem, var, diffVar, neighbor))
			{
				std::vector<std::reference_wrapper<Variable<T>>> diffVariables{ variables };
				__eraseVarAndNeighborFromDiffVariables<T>(diffVariables, var, neighbor);
				for (Variable<T>& diffVar : diffVariables)
				{
					varDiffVarNeighborTriplets.emplace(diffVar, var, neighbor);
					varDiffVarNeighborTriplets.emplace(diffVar, neighbor, var);
				}
			}
		}
		return constraintProblem.isPotentiallySolvable();
	}

	template <typename T>
	static void __eraseVarAndNeighborFromDiffVariables(std::vector<std::reference_wrapper<Variable<T>>>& diffVariables, 
		const Variable<T>& var, const Variable<T>& neighbor)
	{
		for (auto it = diffVariables.begin(); it != diffVariables.cend(); )
		{
			if ((*it).get() == var || (*it).get() == neighbor)
			{
				it = diffVariables.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	
	template <typename T>
	static bool __revise3(ConstraintProblem<T>& constraintProblem, Variable<T>& var, Variable<T>& neighbor, Variable<T>& diffVar)
	{
		bool anyRevised = false;
		const std::vector<T>& varDomain = var.getDomain();
		for (size_t i = 0; i < varDomain.size(); ++i)
		{
			bool varWasAssigned = var.isAssigned();
			if (!varWasAssigned)
			{
				var.assign(varDomain[i]);
			}

			bool currRevised = false;
			const std::vector<T>& neighborDomain = neighbor.getDomain();
			std::vector<T> consistentNeighborValues{ neighborDomain.size() };
			bool neighborWasAssigned = neighbor.isAssigned();

			for (size_t i = 0; i < neighborDomain.size(); ++i)
			{
				if (!neighborWasAssigned)
				{
					neighbor.assign(neighborDomain[i]);
				}
				if (!constraintProblem.getConsistentDomain(diffVar).empty())
				{
					consistentNeighborValues.emplace_back(neighborDomain[i]);
				}
				else
				{
					currRevised = true;
					anyRevised = true;
				}
				if (!neighborWasAssigned)
				{
					neighbor.unassign();
				}
			}

			if (!varWasAssigned)
			{
				var.unassign();
			}
			if (currRevised)
			{
				var.removeFromDomain(i);
				neighbor.setSubsetDomain(consistentNeighborValues);
			}
		}
		return anyRevised;
	}
}