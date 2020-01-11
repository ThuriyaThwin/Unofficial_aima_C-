#include "pch.h"
#include "magic_square_problem.h"


static std::unordered_set<unsigned int> __init_domain(unsigned int order)
{
	std::unordered_set<unsigned int> domain;
	domain.reserve(order);
	for (unsigned int i = 1; i <= order; ++i)
	{
		domain.emplace(i);
	}
	return domain;
}

static void __init_name_and_square_to_var_ref_map(unsigned int order, const std::unordered_set<unsigned int>& domain,
	std::vector<csp::Variable<unsigned int>>& variables,
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>>& squareToVarRefMap,
	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>>& nameToVarRefMap)
{
	variables.reserve(order);
	for (unsigned int i = 1; i <= order; ++i)
	{
		variables.emplace_back(domain);
		squareToVarRefMap.emplace(i, variables.back());
		nameToVarRefMap.emplace(std::to_string(i), variables.back());
	}
}

static void __emplace_all_diff_constraint(unsigned int order, std::vector<csp::Variable<unsigned int>>& variables,
	std::vector<csp::Constraint<unsigned int>>& constraints)
{
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> varRefs;
	varRefs.reserve(order);
	for (csp::Variable<unsigned int>& var : variables)
	{
		varRefs.emplace_back(var);
	}

	constraints.emplace_back(varRefs, csp::allDiff<unsigned int>);
}

static void __emplace_row_constraints(unsigned int n, unsigned int order, 
	csp::ExactLengthExactSum<unsigned int>& exactMagicSumConstrEvaluator,
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>>& squareToVarRefMap,
	std::vector<csp::Constraint<unsigned int>>& constraints)
{
	for (unsigned int row = 1; row <= order; row += n)
	{
		std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> varRefs;
		varRefs.reserve(n);
		for (unsigned int i = row; i < row + n; ++i)
		{
			varRefs.emplace_back(squareToVarRefMap.at(i));
		}
		constraints.emplace_back(varRefs, exactMagicSumConstrEvaluator);
	}
}

static void __emplace_column_constraints(unsigned int n, unsigned int order,
	csp::ExactLengthExactSum<unsigned int>& exactMagicSumConstrEvaluator,
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>>& squareToVarRefMap,
	std::vector<csp::Constraint<unsigned int>>& constraints)
{
	for (unsigned int col = 1; col <= n; ++col)
	{
		std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> varRefs;
		varRefs.reserve(n);
		for (unsigned int i = col; i <= order; i += n)
		{
			varRefs.emplace_back(squareToVarRefMap.at(i));
		}
		constraints.emplace_back(varRefs, exactMagicSumConstrEvaluator);
	}
}

static void __emplace_diagonal_constraints(unsigned int n, unsigned int order,
	csp::ExactLengthExactSum<unsigned int>& exactMagicSumConstrEvaluator,
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>>& squareToVarRefMap,
	std::vector<csp::Constraint<unsigned int>>& constraints)
{
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> firstDiagVarRefs;
	for (unsigned int i = 1; i <= order; i += n + 1)
	{
		firstDiagVarRefs.emplace_back(squareToVarRefMap.at(i));
	}
	constraints.emplace_back(firstDiagVarRefs, exactMagicSumConstrEvaluator);

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> secondDiagVarRefs;
	for (unsigned int i = n; i < order; i += n - 1)
	{
		secondDiagVarRefs.emplace_back(squareToVarRefMap.at(i));
	}
	constraints.emplace_back(secondDiagVarRefs, exactMagicSumConstrEvaluator);
}

csp::ConstraintProblem<unsigned int> constructMagicSquareProblem(unsigned int n, std::vector<csp::Variable<unsigned int>>& variables,
	std::vector<csp::Constraint<unsigned int>>& constraints)
	/*
	number of variables: n x n
	domain of each variable: 1, ..., n x n
	constraints:
		1. global all different.
		2. the values of all rows sum up to magic sum.
		3. the values of all columns sum up to magic sum.
		4. the values of both diagonals sum up to magic sum.
	*/
{
	unsigned int order = n * n;
	unsigned int magicSum = n * ((order + 1) / 2);

	std::unordered_set<unsigned int> domain = __init_domain(order);
	
	std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<unsigned int>>> squareToVarRefMap;
	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>> nameToVarRefMap;
	__init_name_and_square_to_var_ref_map(order, domain, variables, squareToVarRefMap, nameToVarRefMap);
	
	constraints.reserve(order);
	__emplace_all_diff_constraint(order, variables, constraints);

	csp::ExactLengthExactSum<unsigned int> exactMagicSumConstrEvaluator{ n, magicSum };

	__emplace_row_constraints(n, order, exactMagicSumConstrEvaluator, squareToVarRefMap, constraints);
	__emplace_column_constraints(n, order, exactMagicSumConstrEvaluator, squareToVarRefMap, constraints);
	__emplace_diagonal_constraints(n, order, exactMagicSumConstrEvaluator, squareToVarRefMap, constraints);

	std::vector<std::reference_wrapper<csp::Constraint<unsigned int>>> constraintsRefs{ constraints.begin(), constraints.end() };
	csp::ConstraintProblem<unsigned int> magicSquareProblem{ constraintsRefs, nameToVarRefMap };
	return magicSquareProblem;
}