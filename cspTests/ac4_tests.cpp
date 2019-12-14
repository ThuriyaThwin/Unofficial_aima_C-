#include "pch.h"
#include "CppUnitTest.h"
#include <csp.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace cspTests
{
	TEST_CLASS(ac4Tests)
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

		csp::ConstraintProblem<std::string> graphColoringProb{ {constr1, constr2, constr3, constr4, constr5, constr6, constr7, 
			constr8, constr9, constr10} };


		csp::Variable<int> x{ {2, 5} };
		csp::Variable<int> y{ {2, 4} };
		csp::Variable<int> z{ {2, 5} };

		std::function<bool(const std::vector<int>&)> isDivisor = [](const std::vector<int>& values) -> bool
		{
			if (values.size() < 2)
			{
				return true;
			}
			double res = static_cast<double>(values[0]) / values[1];
			return res == static_cast<int>(res);
		};

		csp::Constraint<int> constr11{ {x, z}, isDivisor };
		csp::Constraint<int> constr12{ {y, z}, isDivisor };
		csp::ConstraintProblem<int> constProb1{ {constr11, constr12} };


		csp::Variable<int> s{ {1, 2, 3} };
		csp::Variable<int> t{ {1, 2, 3} };

		std::function<bool(const std::vector<int>&)> lessThan = [](const std::vector<int>& values) -> bool
		{
			if (values.size() < 2)
			{
				return true;
			}
			return values[0] < values[1];
		};

		csp::Constraint<int> constr13{ {s, t}, lessThan };
		csp::ConstraintProblem<int> constProb2{ {constr13} };

		TEST_METHOD_INITIALIZE(ac4TestsSetUp)
		{
			graphColoringProb.unassignAllVariables();
			constProb1.unassignAllVariables();
			constProb2.unassignAllVariables();
		}

		TEST_METHOD(TestOneAC4)
		{
			bool ac4Res = csp::ac4(constProb1);
			Assert::IsTrue(ac4Res);
		}

		TEST_METHOD(TestTwoAC4)
		{
			std::unordered_set<int> actualAllValues;
			for (const csp::Variable<int>& var : constProb1.getVariables())
			{
				for (int val : var.getDomain())
				{
					actualAllValues.insert(val);
				}
			}
			std::unordered_set<int> expectedAllValues{ 2, 4, 5 };
			Assert::IsTrue(actualAllValues == expectedAllValues);
			bool ac4Res = csp::ac4(constProb1);
			Assert::IsTrue(ac4Res);

			std::unordered_set<int> actualReducedValues;
			for (const csp::Variable<int>& var : constProb1.getVariables())
			{
				for (int val : var.getDomain())
				{
					actualReducedValues.insert(val);
				}
			}
			std::unordered_set<int> expectedReducedValues{ 2, 4 };
			Assert::IsTrue(actualReducedValues == expectedReducedValues);
		}

		TEST_METHOD(TestThreeAC4)
		{
			// same as TestThreeAC3
			bool ac4Res = csp::ac4(constProb2);
			Assert::IsTrue(ac4Res);
			for (const csp::Variable<int>& var : constProb2.getVariables())
			{
				const auto& oneFindRes = std::find(var.getDomain().cbegin(), var.getDomain().cend(), 1);
				if (oneFindRes != var.getDomain().cend())
				{
					const auto& twoFindRes = std::find(var.getDomain().cbegin(), var.getDomain().cend(), 2);
					Assert::IsTrue(twoFindRes != var.getDomain().cend());
				}
				else
				{
					const auto& threeFindRes = std::find(var.getDomain().cbegin(), var.getDomain().cend(), 3);
					if (threeFindRes != var.getDomain().cend())
					{
						const auto& twoFindRes = std::find(var.getDomain().cbegin(), var.getDomain().cend(), 2);
						Assert::IsTrue(twoFindRes != var.getDomain().cend());
					}
				}
			}
		}
	};
}