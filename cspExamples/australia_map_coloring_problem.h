#pragma once

#include "pch.h"

csp::ConstraintProblem<std::string> constructAustraliaMapColoringProblem(std::vector<csp::Variable<std::string>>& variables,
	std::vector<csp::Constraint<std::string>>& constraints);