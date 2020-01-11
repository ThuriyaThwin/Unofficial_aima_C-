#include "pch.h"
#include "n_queens_problem.h"


static std::unordered_set<unsigned int> __initDomain(unsigned int n)
{
	std::unordered_set<unsigned int> domain;
	domain.reserve(n);
	for (unsigned int i = 0; i < n; ++i)
	{
		domain.emplace(i);
	}
	return domain;
}

static void __init_name_and_col_to_var_ref_map(unsigned int n, const std::unordered_set<unsigned int>& domain, 
	std::vector<csp::Variable<unsigned int>>& variables,
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>>& columnToVarRefMap,
	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>>& nameToVarRefMap)
{
	variables.reserve(n);
	for (unsigned int i = 0; i < n; ++i)
	{
		variables.emplace_back(domain);
		columnToVarRefMap.emplace(i, variables.back());
		nameToVarRefMap.emplace(std::to_string(i), variables.back());
	}
}

static unsigned int __uintCalcDifference(unsigned int first, unsigned int second)
{
	if (first < second)
	{
		return second - first;
	}
	else
	{
		return first - second;
	}
}

static std::unordered_map<unsigned int, csp::ConstraintEvaluator<unsigned int>> __init_notAttackingConstraintEvaluatorsMap(unsigned int n)
{
	std::unordered_map<unsigned int, csp::ConstraintEvaluator<unsigned int>> notAttackingConstrainEvaluatorsMap;
	for (unsigned int columnDifference = 1; columnDifference < n; ++columnDifference)
	{
		notAttackingConstrainEvaluatorsMap.emplace(columnDifference,
			[columnDifference](const std::vector<unsigned int>& assignedValues) -> bool
			{
				if (assignedValues.size() < 2)
				{
					return true;
				}
				unsigned int firstRow = assignedValues[0];
				unsigned int secondRow = assignedValues[1];
				return firstRow != secondRow && __uintCalcDifference(firstRow, secondRow) != columnDifference;
			});
	}
	return notAttackingConstrainEvaluatorsMap;
}

static void __init_constraints(unsigned int n, std::vector<csp::Constraint<unsigned int>>& constraints,
	const std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>>& columnToVarRefMap,
	std::unordered_map<unsigned int, csp::ConstraintEvaluator<unsigned int>>& notAttackingConstrainEvaluatorsMap)
{
	constraints.reserve(((n * n) + n) / 2);
	for (unsigned int firstCol = 0; firstCol < n; ++firstCol)
	{
		for (unsigned int secondCol = 0; secondCol < n; ++secondCol)
		{
			if (firstCol < secondCol)
			{
				std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> varRefs
				{
					std::reference_wrapper<csp::Variable<unsigned int>>{ columnToVarRefMap.at(firstCol) },
					std::reference_wrapper<csp::Variable<unsigned int>>{ columnToVarRefMap.at(secondCol) }
				};
				constraints.emplace_back(varRefs,
					notAttackingConstrainEvaluatorsMap[__uintCalcDifference(firstCol, secondCol)]);
			}
		}
	}
}

csp::ConstraintProblem<unsigned int> constructNQueensProblem(unsigned int n, std::vector<csp::Variable<unsigned int>>& variables,
	std::vector<csp::Constraint<unsigned int>>& constraints)
	/*
	SOLVING N-QUEENS PROBLEM:
	Each variable represent a column in the n x n sized board.
	Each variable's domain is (0, ..., n - 1), and it represent a possible queen row-coordinate location in the column i.
	For the queens not to attack each other, the constraints are:
		1. No single row hold two queens: all variables are (pair-wise) all-different.
		2. The queens don't attack each other horizontally.
		3. The queens don't attack each other diagonally.
	*/
{
	std::unordered_set<unsigned int> domain = __initDomain(n);
	
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>> columnToVarRefMap;
	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>> nameToVarRefMap;
	__init_name_and_col_to_var_ref_map(n, domain, variables, columnToVarRefMap, nameToVarRefMap);

	std::unordered_map<unsigned int, csp::ConstraintEvaluator<unsigned int>> notAttackingConstrainEvaluatorsMap = __init_notAttackingConstraintEvaluatorsMap(n);

	__init_constraints(n, constraints, columnToVarRefMap, notAttackingConstrainEvaluatorsMap);
	
	std::vector<std::reference_wrapper<csp::Constraint<unsigned int>>> constraintsRefs{ constraints.begin(), constraints.end() };
	csp::ConstraintProblem<unsigned int> nQueensProblem{ constraintsRefs, nameToVarRefMap };
	return nQueensProblem;
}