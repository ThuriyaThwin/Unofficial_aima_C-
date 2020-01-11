#pragma once

#include "pch.h"
#include "tree_csp_solver.h"


/*
///////////////////////////////////////// Cutset Conditioning Algorithm /////////////////////////////////////////
1. Find a cycle cutset, i.e. a set of variables that once removed from the constraint graph, the graph becomes a tree.
2. For each possible assignment to the cycle cutset variables which is consistent within their constraints:
   I. Remove from the domains of the remaining variables any values that are inconsistent with the cycle cutset's assignment.
   II. If the remaining CSP has a solution, return it together with the assignment for cycle cutset.


How should we find a cycle cutset? Finding a minimal cycle cutset is a NP-hard problem. Iterative compression might be the best option.
Yet we don't require a minimal cycle cutset, just a cycle cutset.
Ergo a naive algorithm is implemented:
 1. Define the length of a constraint to be the number of its variables.
 2. Sort the constrains by their lengths in descending order (longer to shorter).
 3. Find all of the variables of the first (longest) constraint.
 4. Remove these variables from the graph, and see if it becomes a tree.
 5. If it is a tree:
 6.    Go to step 2 in the aforementioned (upper) algorithm
 7. Else:
 8.    Go to step 3, but this time use the variables of the second-longest constraint, and so on and so forth...
 NOTE: This naive algorithm is incomplete (does not guarantee to find a solution).
 /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/


namespace csp
{
	template <typename T>
	using ConstraintGraph = std::unordered_map<Ref<Variable<T>>, std::vector<Ref<Variable<T>>>>;

	template <typename T>
	static bool __isCyclicGraph(ConstraintGraph<T>& reducedGraph,
		std::unordered_set<Ref<Variable<T>>>& visitedNodes,
		Variable<T>& node, std::optional<Variable<T>> parent)
	{
		visitedNodes.emplace(node);
		for (Variable<T>& neighbor : reducedGraph.at(node))
		{
			if (!visitedNodes.count(neighbor))
			{
				if (__isCyclicGraph<T>(reducedGraph, visitedNodes, neighbor, node))
				{
					return true;
				}
			}
			else if (neighbor != parent)
			{
				return true;
			}
		}
		return false;
	}

	template <typename T>
	static bool __isTree(ConstraintGraph<T>& reducedGraph)
	{
		/* By definition a tree is an acyclic connected graph */
		if (reducedGraph.empty())
		{
			return false;
		}

		std::unordered_set<Ref<Variable<T>>> visitedNodes;
		std::vector<Ref<Variable<T>>> keys;
		for (const auto& elem : reducedGraph)
		{
			keys.emplace_back(elem.first);
		}
		Variable<T>& randomlySelectedRoot = __selectElementRandomly<Ref<Variable<T>>, std::vector<Ref<Variable<T>>>>(keys);
		if (__isCyclicGraph<T>(reducedGraph, visitedNodes, randomlySelectedRoot, std::optional<Variable<T>>{ }))
		{
			return false;
		}

		std::unordered_set<Ref<Variable<T>>> reducedGraphVars;
		// std::unordered_map<Ref<Variable<T>>, std::vector<Ref<Variable<T>>>>;
		for (std::pair<const Ref<Variable<T>>, std::vector<Ref<Variable<T>>>>& varToNeighbors : reducedGraph)
		{
			reducedGraphVars.emplace(varToNeighbors.first);
			reducedGraphVars.insert(varToNeighbors.second.begin(), varToNeighbors.second.end());
		}

		bool isConnected = visitedNodes.size() == reducedGraphVars.size();
		return isConnected;
	}

	// taken from: https://gist.github.com/thirdwing/953b146ba39c5f5ff562
	// another alternative: https://gist.github.com/Alexhuszagh/67efb078a82616ed07529ed97586646a
	template <typename T>
	static std::vector<std::vector<T>> __cartesian_product(const std::vector<std::vector<T>>& v) {
		std::vector<std::vector<T>> s = { {} };
		for (auto& u : v) {
			std::vector<std::vector<T>> r;
			for (auto& x : s) {
				for (auto y : u) {
					r.push_back(x);
					r.back().push_back(y);
				}
			}
			s.swap(r);
		}
		return s;
	}

	template <typename T>
	static std::vector<std::vector<T>> __getConsistentAssignmentsValues(
		const std::unordered_set<Ref<Variable<T>>>& cutSetVars,
		const std::vector<Ref<Constraint<T>>>& cutSetConstraints,
		const std::unordered_set<Ref<Variable<T>>>& readOnlyVariables)
	{
		std::vector<std::vector<T>> domains;
		domains.reserve(cutSetVars.size());
		for (const Variable<T> var : cutSetVars)
		{
			domains.emplace_back(var.getDomain());
		}
		std::vector<std::vector<T>> domainsProduct = __cartesian_product<T>(domains);
		std::vector<std::vector<T>> consistentAssignmentValues;
		for (const std::vector<T>& assignmentValues : domainsProduct)
		{
			std::vector<std::pair<Ref<Variable<T>>, T>> vecVarToValue;
			std::transform(cutSetVars.cbegin(), cutSetVars.cend(), assignmentValues.cbegin(), 
				std::back_inserter(vecVarToValue), [](const Ref<Variable<T>>& var, PassType<T> value) -> std::pair<Ref<Variable<T>>, T>
				{
					return std::make_pair(var, value);
				} );

			for (const std::pair<Ref<Variable<T>>, T>& varToValue : vecVarToValue)
			{
				if (!readOnlyVariables.count(varToValue.first))
				{
					varToValue.first.get().assignByValue(varToValue.second);
				}
			}

			bool allCutSetConstraintsAreSatisfied = true;
			for (const Constraint<T>& constr : cutSetConstraints)
			{
				if (!constr.isSatisfied())
				{
					allCutSetConstraintsAreSatisfied = false;
					break;
				}
			}

			if (allCutSetConstraintsAreSatisfied)
			{
				consistentAssignmentValues.emplace_back(assignmentValues);
			}

			for (Variable<T>& var : cutSetVars)
			{
				if (!readOnlyVariables.count(var))
				{
					var.unassign();
				}
			}
		}

		return consistentAssignmentValues;
	}

	template <typename T>
	const AssignmentHistory<T> naiveCycleCutset(ConstraintProblem<T>& constraintProblem, bool writeAssignmentHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		const std::vector<Ref<Variable<T>>> variables = constraintProblem.getUnassignedVariables();
		const std::vector<Ref<Variable<T>>> vecReadOnlyVariables = constraintProblem.getAssignedVariables();
		const std::unordered_set<Ref<Variable<T>>> readOnlyVariables{ vecReadOnlyVariables.cbegin(), vecReadOnlyVariables.cend() };
		std::vector<Ref<Constraint<T>>>& constraints = 
			const_cast<std::vector<Ref<Constraint<T>>>&>(constraintProblem.getConstraints());
		std::sort(std::execution::par_unseq, constraints.begin(), constraints.end(),
			[] (const Constraint<T>& left, const Constraint<T>& right) -> bool
			{
				return left.getVariables().size() > right.getVariables().size();
			});
		const ConstraintGraph<T>& constraintGraph = constraintProblem.getConstraintGraph();

		const auto constraintsItToBegin = constraints.cbegin();


		for (size_t i = 1; i < constraints.size(); ++i)
		{
			std::vector<Ref<Constraint<T>>> cutSetConstraints{ constraintsItToBegin, constraintsItToBegin + i };
			std::unordered_set<Ref<Variable<T>>> cutSetVars;
			for (const Constraint<T>& constr : constraints)
			{
				const std::vector<Ref<Variable<T>>>& constraintVars = constr.getVariables();
				cutSetVars.insert(constraintVars.cbegin(), constraintVars.cend());
			}

			ConstraintGraph<T> reducedGraph;
			reducedGraph.reserve(variables.size());
			for (auto& varToNeighbors : constraintGraph)
			{
				Variable<T>& var = varToNeighbors.first;
				if (!cutSetVars.count(var))
				{
					reducedGraph.try_emplace(var, std::vector<Ref<Variable<T>>>{});
					reducedGraph.at(var).reserve(constraintGraph.at(var).size());
					for (Variable<T>& neighbor : varToNeighbors.second)
					{
						if (!cutSetVars.count(neighbor))
						{
							reducedGraph.at(var).emplace_back(neighbor);
						}
					}
				}
			}

			if (__isTree<T>(reducedGraph))
			{
				std::vector<std::vector<T>> cosistentAssignmentsValyes = __getConsistentAssignmentsValues<T>(cutSetVars, 
					cutSetConstraints, readOnlyVariables);
				std::vector<Ref<Variable<T>>> nonCutsetVars;
				nonCutsetVars.reserve(variables.size());
				for (Variable<T>& var : variables)
				{
					if (!cutSetVars.count(var))
					{
						nonCutsetVars.emplace_back(var);
					}
				}

				std::unordered_map<Ref<Variable<T>>, std::vector<T>> nonCutSetVarsToOriginalDomains;
				for (Variable<T>& var : nonCutsetVars)
				{
					nonCutSetVarsToOriginalDomains.emplace(var, var.getDomain());
				}

				for (const std::vector<T>& consistentAssignmentValues : cosistentAssignmentsValyes)
				{
					std::vector<std::pair<Ref<Variable<T>>, T>> vecVarToValue;
					std::transform(cutSetVars.cbegin(), cutSetVars.cend(), consistentAssignmentValues.cbegin(),
						std::back_inserter(vecVarToValue), [](Variable<T>& var, PassType<T> value) -> std::pair<Ref<Variable<T>>, PassType<T>>
						{
							return std::pair<Ref<Variable<T>>, T>{ var, value};
						});

					for (const std::pair<Ref<Variable<T>>, T>& varToValue : vecVarToValue)
					{
						if (!readOnlyVariables.count(varToValue.first))
						{
							varToValue.first.get().assignByValue(varToValue.second);
							if (writeAssignmentHistory)
							{
								// CSPDO: fix it
								//assignmentHistory.emplace_back(varToValue.first, varToValue.first.get().getAssignmentIdx());
							}	
						}
					}
					for (Variable<T>& nonCutSetVariable : nonCutsetVars)
					{
						if (!readOnlyVariables.count(nonCutSetVariable))
						{
							nonCutSetVariable.setSubsetDomain(constraintProblem.getConsistentDomain(nonCutSetVariable));
						}
					}

					AssignmentHistory<T> treeCSPAssignmentHistory = treeCspSolver(constraintProblem, writeAssignmentHistory);
					if (writeAssignmentHistory)
					{
						// CSPDO: fix it
						//assignmentHistory.insert(treeCSPAssignmentHistory.cbegin(), treeCSPAssignmentHistory.cend());
					}
					if (constraintProblem.isCompletelyConsistentlyAssigned())
					{
						return assignmentHistory;
					}

					for (Variable<T>& var : variables)
					{
						if (!readOnlyVariables.count(var))
						{
							var.unassign();
						}
						if (writeAssignmentHistory)
						{
							assignmentHistory.emplace_back(var, std::optional<T>{ });
						}
					}

					for (std::pair<Ref<Variable<T>>, std::vector<T>> varToOriginalDomain : nonCutSetVarsToOriginalDomains)
					{
						if (!readOnlyVariables.count(varToOriginalDomain.first))
						{
							varToOriginalDomain.first.get().setDomain(varToOriginalDomain.second);
						}
					}
				}

			}

		}


		return assignmentHistory;
	}
}