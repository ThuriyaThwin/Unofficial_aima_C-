#include "pch.h"
#include "CppUnitTest.h"
#include "heuristic_backtracking.h"
#include "constraint_evaluators.h"
#include "domain_sorters.h"
#include "unassigned_variable_selectors.h"
#include "forward_checking.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace cspTests
{
	TEST_CLASS(HeuristicBacktrackingTests)
	{
	public:

		const std::unordered_set<std::string> domain{ "Red", "Green", "Blue" };
		const std::unordered_set<std::string> names{ "nt", "q", "nsw", "v", "t", "sa", "wa" };
		std::unordered_map<std::string, csp::Variable<std::string>> NameToVarUMap = csp::Variable<std::string>::constructFromNamesToEqualDomain(names, domain);

		csp::Constraint<std::string> constr1{ {NameToVarUMap.at("sa"), NameToVarUMap.at("wa")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr2{ {NameToVarUMap.at("sa"), NameToVarUMap.at("nt")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr3{ {NameToVarUMap.at("sa"), NameToVarUMap.at("q")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr4{ {NameToVarUMap.at("sa"), NameToVarUMap.at("nsw")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr5{ {NameToVarUMap.at("sa"), NameToVarUMap.at("v")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr6{ {NameToVarUMap.at("wa"), NameToVarUMap.at("nt")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr7{ {NameToVarUMap.at("nt"), NameToVarUMap.at("q")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr8{ {NameToVarUMap.at("q"), NameToVarUMap.at("nsw")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr9{ {NameToVarUMap.at("nsw"), NameToVarUMap.at("v")}, csp::allDiff<std::string> };
		csp::Constraint<std::string> constr10{ {NameToVarUMap.at("t")}, [](const std::vector<std::string>& values) -> bool {return true; } };

		csp::ConstraintProblem<std::string> graphColoringProb{ {constr1, constr2, constr3, constr4, constr5, constr6, constr7, constr8,
			constr9, constr10} };

		TEST_METHOD_INITIALIZE(HeuristicBacktrackingTestsSetUp)
		{
			graphColoringProb.unassignAllVariables();
		}

		TEST_METHOD(TestClassicHeuristicBacktracking)
		{
			const csp::AssignmentHistory<std::string>& assignmentHistory = csp::heuristicBacktrackingSolver<std::string>(graphColoringProb,
				csp::minimumRemainingValues_primarySelector<std::string>,
				csp::degreeHeuristic_secondarySelector<std::string>,
				csp::leastConstrainingValue<std::string>,
				csp::forwardChecking<std::string>);
			Assert::IsTrue(graphColoringProb.isCompletelyConsistentlyAssigned());
		}
	};
}


// TODO:
// try using emplace_back instead of push_back everywhere