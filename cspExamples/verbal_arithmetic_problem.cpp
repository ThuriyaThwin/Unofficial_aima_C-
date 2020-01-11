#include "pch.h"
#include "verbal_arithmetic_problem.h"


static std::unordered_set<unsigned int> allDigitsDomain		{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static std::unordered_set<unsigned int> noZeroDigitsDomain	{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static std::unordered_set<unsigned int> carryDigitsDomain	{ 0, 1 };
static std::unordered_map<std::string, std::unordered_set<unsigned int>> nameToDomainMap;
static std::unordered_set<std::string> carryDigitsNames{ "c_10", "c_100", "c_1000" };

static void __init_nameToDomainMap()
{
	nameToDomainMap["o"] = allDigitsDomain;
	nameToDomainMap["w"] = allDigitsDomain;
	nameToDomainMap["r"] = allDigitsDomain;
	nameToDomainMap["u"] = allDigitsDomain;

	nameToDomainMap["t"] = noZeroDigitsDomain;
	nameToDomainMap["f"] = noZeroDigitsDomain;

	nameToDomainMap["c_10"] = carryDigitsDomain;
	nameToDomainMap["c_100"] = carryDigitsDomain;
	nameToDomainMap["c_1000"] = carryDigitsDomain;
}

static std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> __init_uniqueDigitsConstraintVarRefs(
	const std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>>& nameToVarRefMap)
{
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> uniqueDigitsConstraintVarRefs;
	uniqueDigitsConstraintVarRefs.reserve(6);
	for (const std::pair<std::string, std::reference_wrapper<csp::Variable<unsigned int>>>& nameToVarRef : nameToVarRefMap)
	{
		if (carryDigitsNames.count(nameToVarRef.first))
		{
			continue;
		}
		uniqueDigitsConstraintVarRefs.emplace_back(nameToVarRef.second);
	}
	return uniqueDigitsConstraintVarRefs;
}

static bool unitsDigitConstraintEvaluator(const std::vector<unsigned int>& assignedValues)
{
	if (assignedValues.size() < 3)
	{
		return true;
	}
	unsigned int o = assignedValues[0];
	unsigned int r = assignedValues[1];
	unsigned int c_10 = assignedValues[2];
	return o + o == r + 10 * c_10;
};

static bool tensDigitConstraintEvaluator(const std::vector<unsigned int>& assignedValues)
{
	if (assignedValues.size() < 4)
	{
		return true;
	}
	unsigned int c_10 = assignedValues[0];
	unsigned int w = assignedValues[1];
	unsigned int u = assignedValues[2];
	unsigned int c_100 = assignedValues[3];
	return c_10 + w + w == u + 10 * c_100;
};

static bool hundredsDigitConstraintEvaluator(const std::vector<unsigned int>& assignedValues)
{
	if (assignedValues.size() < 4)
	{
		return true;
	}
	unsigned int c_100 = assignedValues[0];
	unsigned int t = assignedValues[1];
	unsigned int o = assignedValues[2];
	unsigned int c_1000 = assignedValues[3];
	return c_100 + t + t == o + 10 * c_1000;
};

static bool thousandsDigitConstraintEvaluator(const std::vector<unsigned int>& assignedValues)
{
	if (assignedValues.size() < 2)
	{
		return true;
	}
	unsigned int c_1000 = assignedValues[0];
	unsigned int f = assignedValues[1];
	return c_1000 == f;
};

csp::ConstraintProblem<unsigned int> constructVerbalArithmeticProblem(std::vector<csp::Variable<unsigned int>>& variables,
	std::vector<csp::Constraint<unsigned int>>& constraints)
	/*
	SOLVING:                      (where each letter uniquely represent a digit)
	               TWO
	              +TWO
	              ----
	              FOUR
	*/
{
	__init_nameToDomainMap();

	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>> nameToVarRefMap =
		csp::Variable<unsigned int>::constructFromNamesToDomainsPutInVec(nameToDomainMap, variables);

	
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> uniqueDigitsConstraintVarRefs = 
		__init_uniqueDigitsConstraintVarRefs(nameToVarRefMap);
	

	constraints.reserve(5);
	constraints.emplace_back(uniqueDigitsConstraintVarRefs, csp::allDiff<unsigned int>);

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> unitsDigitsConstraintVarRefs
	{
		nameToVarRefMap.at("o"), nameToVarRefMap.at("r"), nameToVarRefMap.at("c_10")
	};
	constraints.emplace_back(unitsDigitsConstraintVarRefs, unitsDigitConstraintEvaluator);

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> tensDigitsConstraintVarRefs
	{
		nameToVarRefMap.at("c_10"), nameToVarRefMap.at("w"), nameToVarRefMap.at("u"), nameToVarRefMap.at("c_100")
	};
	constraints.emplace_back(tensDigitsConstraintVarRefs, tensDigitConstraintEvaluator);

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> hundredsDigitsConstraintVarRefs
	{
		nameToVarRefMap.at("c_100"), nameToVarRefMap.at("t"), nameToVarRefMap.at("o"), nameToVarRefMap.at("c_1000")
	};
	constraints.emplace_back(hundredsDigitsConstraintVarRefs, hundredsDigitConstraintEvaluator);

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> thousandsDigitsConstraintVarRefs
	{
		nameToVarRefMap.at("c_1000"), nameToVarRefMap.at("f")
	};
	constraints.emplace_back(thousandsDigitsConstraintVarRefs, thousandsDigitConstraintEvaluator);

	std::vector<std::reference_wrapper<csp::Constraint<unsigned int>>> constraintsRefs{ constraints.begin(), constraints.end() };
	csp::ConstraintProblem<unsigned int> verbalArithmeticProblem{ constraintsRefs, nameToVarRefMap };
	return verbalArithmeticProblem;
}