#pragma once
#include "pch.h"


struct VarRefsMaps
{
    std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<std::string>>> colorsVarsMap;
    std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<std::string>>> nationalitiesVarsMap;
    std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<std::string>>> drinksVarsMap;
    std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<std::string>>> smokesVarsMap;
    std::unordered_map<unsigned int, std::reference_wrapper<csp::Variable<std::string>>> petsVarsMap;
};

csp::ConstraintProblem<std::string> constructEinsteinRiddleProblem(std::vector<csp::Variable<std::string>>& variables,
	std::vector<csp::Constraint<std::string>>& constraints, VarRefsMaps& varRefsMaps);