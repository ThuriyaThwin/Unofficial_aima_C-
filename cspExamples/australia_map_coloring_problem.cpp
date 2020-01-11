#include "pch.h"
#include "australia_map_coloring_problem.h"


csp::ConstraintProblem<std::string> constructAustraliaMapColoringProblem(std::vector<csp::Variable<std::string>>& variables,
	std::vector<csp::Constraint<std::string>>& constraints)
{
	const std::unordered_set<std::string> domain{ "Red", "Green", "Blue" };
	const std::unordered_set<std::string> names{ "nt", "q", "nsw", "v", "t", "sa", "wa" };
	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<std::string>>> nameToVarRefMap =
		csp::Variable<std::string>::constructFromNamesToEqualDomainPutInVec(names, domain, variables);

	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr1Vars{ nameToVarRefMap.at("sa"), nameToVarRefMap.at("wa") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr2Vars{ nameToVarRefMap.at("sa"), nameToVarRefMap.at("nt") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr3Vars{ nameToVarRefMap.at("sa"), nameToVarRefMap.at("q") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr4Vars{ nameToVarRefMap.at("sa"), nameToVarRefMap.at("nsw") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr5Vars{ nameToVarRefMap.at("sa"), nameToVarRefMap.at("v") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr6Vars{ nameToVarRefMap.at("wa"), nameToVarRefMap.at("nt") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr7Vars{ nameToVarRefMap.at("nt"), nameToVarRefMap.at("q") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr8Vars{ nameToVarRefMap.at("q"), nameToVarRefMap.at("nsw") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr9Vars{ nameToVarRefMap.at("nsw"), nameToVarRefMap.at("v") };
	std::vector<std::reference_wrapper<csp::Variable<std::string>>> constr10Vars{ nameToVarRefMap.at("t") };

	constraints.reserve(10);
	constraints.emplace_back(constr1Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr2Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr3Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr4Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr5Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr6Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr7Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr8Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr9Vars, csp::allDiff<std::string>);
	constraints.emplace_back(constr10Vars, csp::allDiff<std::string>);

	std::vector<std::reference_wrapper<csp::Constraint<std::string>>> constraintsRef{ constraints.begin(), constraints.end() };
	csp::ConstraintProblem<std::string> graphColoringProb{ constraintsRef, nameToVarRefMap };
	return graphColoringProb;
}