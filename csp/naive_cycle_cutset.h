#pragma once

#include "pch.h"
#include "constraint_problem.h"


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
	const AssignmentHistory<T> naiveCycleCutset(ConstraintProblem<T>& constraintProblem, bool writeAssignmentHistory = false)
	{
		AssignmentHistory<T> assignmentHistory;
		const std::vector<Ref<Variable<T>>>& variables = constraintProblem.getUnassignedVariables();
		const std::vector<Ref<Variable<T>>>& readOnlyVariables = constraintProblem.getAssignedVariables();
		std::vector<Ref<Constraint<T>>>& constraints = 
			const_cast<std::vector<Ref<Constraint<T>>>&>(constraintProblem.getConstraints());
		std::sort(constraints.begin(), constraints.end(), 
			[] (const Constraint<T>& left, const Constraint<T>& right) -> bool
			{
				return left.getVariables().size() > right.getVariables().size();
			});
		const ConstraintGraph<T>& constraintGraph = constraintProblem.getConstraintGraph();

		const auto& constraintsItToBegin = constraints.cbegin();


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
			for (const auto& varToNeighbors : constraintGraph)
			{
				Variable<T>& var = varToNeighbors.first;
				if (!cutSetVars.count(var))
				{
					reducedGraph.try_emplace(var, std::vector<Ref<Variable<T>>>{});
					reducedGraph[var].reserve(constraintGraph.at(var).size());
					for (Variable<T>& neighbor : varToNeighbors.second)
					{
						if (!cutSetVars.count(neighbor))
						{
							reducedGraph[var].emplace_back(neighbor);
						}
					}
				}
			}

			if (__isTree<T>(reducedGraph))
			{

			}

		}


		return assignmentHistory;
	}
}


namespace csp
{
	template <typename T>
	static bool __isTree(const ConstraintGraph<T>& reducedGraph)
	{
		/* By definition a tree is an acyclic connected graph */
		if (reducedGraph.empty())
		{
			return false;
		}

		std::unordered_set<Ref<Variable<T>>> visitedNodes;
		// CSPDO: select root node randomly, call __isCyclic
		

		std::unordered_set<Ref<Variable<T>>> reducedGraphVars;
		for (const auto& varToNeighbors : reducedGraph)
		{
			reducedGraphVars.emplace(varToNeighbors.first);
			std::unordered_set<Ref<Variable<T>>>& neighbors = varToNeighbors.second;
			reducedGraph.insert(neighbors.cbegin(), neighbors.cend())
		}

		bool isConnected = visitedNodes.size() == reducedGraphVars.size();
		return isConnected;
	}

	template <typename T>
	static bool __isCyclicGraph(const ConstraintGraph<T>& reducedGraph, 
		std::unordered_set<Ref<Variable<T>>>& visitedNodes,
		const Variable<T>& node, std::optional<Ref<Variable<T>>> parent)
	{
		visitedNodes.emplace(node);
		for (const Variable<T>& neighbor : reducedGraph[node])
		{
			if (!visitedNodes.count(neighbor))
			{
				if (__isCyclicGraph<T>(reducedGraph, neighbor, node))
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
}


namespace csp
{
	template <typename T>
	static std::vector<Assignment<T>> __getConsistentAssignments(
		const std::unordered_set<Ref<Variable<T>>>& cutSetVars,
		const std::vector<Ref<Constraint<T>>>& cutSetConstraints,
		const std::vector<Ref<Variable<T>>>& readOnlyVariables)
	{
		std::vector<std::vector<T>> domains;
		domains.reserve(cutSetVars.size());
		for (const Variable<T> var : cutSetVars)
		{
			domains.emplace_back(var.getDomain());
		}
		
		std::vector<Assignment<T>> consistentAssignments;
 
	}
}