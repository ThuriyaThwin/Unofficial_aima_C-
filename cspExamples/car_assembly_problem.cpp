#include "pch.h"
#include "car_assembly_problem.h"


static std::unordered_set<unsigned int> __init_domain()
{
	std::unordered_set<unsigned int> domain;
	for (unsigned int i = 1; i < 28; ++i)
	{
		domain.emplace(i);
	}
	return domain;
}

csp::ConstraintProblem<unsigned int> constructCarAssemblyProblem(std::vector<csp::Variable<unsigned int>>& variables,
	std::vector<csp::Constraint<unsigned int>>& constraints)
	/*
	SOLVING JOB-SCHEDULING PROBLEM:
	We'll break down a car assembly operation to 15 tasks, which we'll model as variables:
	[1-2] install axles (front and back),
	[3-6] affix all four wheels (right and left, front and back),
	[7-10] tighten nuts for each wheel,
	[11-14] affix hubcaps,
	[15] inspect the final assembly.
	
	we'll model the following requirement as a domain:
	1. the whole assembly must be done in 30 minutes.
	
	we'll model the following requirements as constraints:
	1. the axles have to be in place before the wheels are put on, and it takes 10 minutes to install an axle.
	2. for each wheel we must affix the wheel (which takes 1 minute), then tighten the nuts (2 minutes), and only then
	    attach the hubcap (which also takes 1 minute).
	3. we have four workers to install wheels, but they have to share one tool that helps put the axle in place.
	    therefore installing the front axle and the back axle cannot be simultaneous.
	4. before shipping the car we must inspect it, and the inspection takes 3 minutes.
	*/
{
	csp::TimeDelayer<unsigned int> oneDelayer{ 1 };
	csp::TimeDelayer<unsigned int> twoDelayer{ 2 };
	csp::TimeDelayer<unsigned int> threeDelayer{ 3 };
	csp::TimeDelayer<unsigned int> tenDelayer{ 10 };

	std::unordered_set<unsigned int> domain = __init_domain();

	// construct variables
	const std::unordered_set<std::string> names{ "axel_f", "axel_b", "wheel_rf", "wheel_lf", "wheel_rb", "wheel_lb", "nuts_rf", 
		"nuts_lf", "nuts_rb", "nuts_lb", "cap_rf", "cap_lf", "cap_rb", "cap_lb", "inspect" };
	std::unordered_map<std::string, std::reference_wrapper<csp::Variable<unsigned int>>> nameToVarRefMap =
		csp::Variable<unsigned int>::constructFromNamesToEqualDomainPutInVec(names, domain, variables);
	// ----------------------------------------------------------------------------------------------------------------------------------------

	// construct constraints
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr1Vars{ nameToVarRefMap.at("axel_f"), nameToVarRefMap.at("wheel_rf") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr2Vars{ nameToVarRefMap.at("axel_b"), nameToVarRefMap.at("wheel_rb") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr3Vars{ nameToVarRefMap.at("axel_f"), nameToVarRefMap.at("wheel_lf") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr4Vars{ nameToVarRefMap.at("axel_b"), nameToVarRefMap.at("wheel_lb") };

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr5Vars{ nameToVarRefMap.at("wheel_rf"), nameToVarRefMap.at("nuts_rf") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr6Vars{ nameToVarRefMap.at("wheel_lf"), nameToVarRefMap.at("nuts_lf") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr7Vars{ nameToVarRefMap.at("wheel_rb"), nameToVarRefMap.at("nuts_rb") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr8Vars{ nameToVarRefMap.at("wheel_lb"), nameToVarRefMap.at("nuts_lb") };

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr9Vars{ nameToVarRefMap.at("nuts_rf"), nameToVarRefMap.at("cap_rf") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr10Vars{ nameToVarRefMap.at("nuts_lf"), nameToVarRefMap.at("cap_lf") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr11Vars{ nameToVarRefMap.at("nuts_rb"), nameToVarRefMap.at("cap_rb") };
	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr12Vars{ nameToVarRefMap.at("nuts_lb"), nameToVarRefMap.at("cap_lb") };

	std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constr13Vars{ nameToVarRefMap.at("axel_f"), nameToVarRefMap.at("axel_b") };

	constraints.reserve(13);
	constraints.emplace_back(constr1Vars, tenDelayer);
	constraints.emplace_back(constr2Vars, tenDelayer);
	constraints.emplace_back(constr3Vars, tenDelayer);
	constraints.emplace_back(constr4Vars, tenDelayer);

	constraints.emplace_back(constr5Vars, oneDelayer);
	constraints.emplace_back(constr6Vars, oneDelayer);
	constraints.emplace_back(constr7Vars, oneDelayer);
	constraints.emplace_back(constr8Vars, oneDelayer);

	constraints.emplace_back(constr9Vars, twoDelayer);
	constraints.emplace_back(constr10Vars, twoDelayer);
	constraints.emplace_back(constr11Vars, twoDelayer);
	constraints.emplace_back(constr12Vars, twoDelayer);

	constraints.emplace_back(constr13Vars, tenDelayer);

	for (const std::string& name : names)
	{
		if (name == "inspect")
		{
			continue;
		}

		std::vector<std::reference_wrapper<csp::Variable<unsigned int>>> constrVarRefs
		{
			nameToVarRefMap.at(name), nameToVarRefMap.at("inspect")
		};

		constraints.emplace_back(constrVarRefs, threeDelayer);
	}

	std::vector<std::reference_wrapper<csp::Constraint<unsigned int>>> constraintsRef{ constraints.begin(), constraints.end() };
	csp::ConstraintProblem<unsigned int> carAssemblyProb{ constraintsRef, nameToVarRefMap };
	return carAssemblyProb;
}