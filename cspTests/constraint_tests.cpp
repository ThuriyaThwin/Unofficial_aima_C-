#include "pch.h"
#include "CppUnitTest.h"
#include <csp.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


bool doubleAllEqual(const std::vector<double>& values)
{
	if (values.empty())
	{
		return false;
	}

	double value = values.front();
	for (double val : values)
	{
		if (val != value)
		{
			return false;
		}
	}
	return true;
}

bool lessThanFive(const std::vector<double>& myValues)
{
	for (double myVal : myValues)
	{
		if (5 < myVal)
		{
			return false;
		}
	}
	return true;
}


namespace cspTests
{
	TEST_CLASS(ConstraintTests)
	{
	public:
		std::unordered_set<double> domain1{ 1.0, 2.5, 3.7, 4.2, 5.5, 6.6, 7.8, 8.8, 9.9, 10 };
		csp::Variable<double> var1{ domain1 };
		csp::Variable<double> var17{ var1 };

		std::unordered_set<double> domain2{ 1.0, 2.5, 3.7, 4.2, 5.5, 6.6 };
		csp::Variable<double> var2{ domain2 };

		std::unordered_set<double> domain3{ 1.0, 2.5, 3.7 };
		csp::Variable<double> var3{ domain3 };

		std::vector<std::reference_wrapper<csp::Variable<double>>> vars{ var1, var2, var3 };
		std::vector<std::reference_wrapper<csp::Variable<double>>> vars7{ var17 };

		csp::Constraint<double> constraint1{ vars, [](const std::vector<double>& values) -> bool {return true; } };

		csp::Constraint<double> constraint2{ vars, doubleAllEqual };

		
		csp::Constraint<double> constraint7{ vars7, lessThanFive };
		


		TEST_METHOD_INITIALIZE(ConstraintSetUp)
		{
			var1.unassign();
			var2.unassign();
			var3.unassign();
		}

		TEST_METHOD(TestCopyCtor)
		{
			std::vector<csp::Constraint<double>> vec;
			csp::Constraint<double> constraint5{ vars, doubleAllEqual };
			vec.push_back(constraint5);
			Assert::IsTrue(vec.size() == 1);
			Assert::IsTrue(vec.back() != constraint5);
		}

		TEST_METHOD(TestCopyAssignmentOperator)
		{
			csp::Constraint<double> constraint5{ vars, doubleAllEqual };
			csp::Constraint<double> constraint6{ vars7, doubleAllEqual };
			constraint5 = constraint6;
			Assert::IsTrue(constraint5.getVariables().size() == 1);
		}

		TEST_METHOD(TestMoveCtor)
		{
			csp::Constraint<double> constraint7{ vars, doubleAllEqual };
			csp::Constraint<double> constraint8{ std::move(constraint7) };
			Assert::IsTrue(constraint7.getVariables().empty());
			Assert::IsTrue(constraint8.getVariables().size() == 3);
		}

		TEST_METHOD(TestMoveAssignmentOperator)
		{
			csp::Constraint<double> constraint5{ vars, doubleAllEqual };
			csp::Constraint<double> constraint6{ vars7, doubleAllEqual };
			constraint5 = std::move(constraint6);
			Assert::IsTrue(constraint5.getVariables().size() == 1);
			Assert::IsTrue(constraint6.getVariables().size() == 3);
		}

		TEST_METHOD(TestgetVariables)
		{
			int i = 0;
			for (const csp::Variable<double>& var : constraint1.getVariables())
			{
				const csp::Variable<double>& otherVar = vars.at(i);
				const std::vector<double>& otherDomain = otherVar.getDomain();
				const std::vector<double>& domain = var.getDomain();
				size_t len = domain.size();
				for (size_t j = 0; j < len; ++j)
				{
					Assert::AreEqual(domain.at(j), otherDomain.at(j));
				}
				++i;
			}
		}

		TEST_METHOD(TestIsCompletelyAssigned)
		{
			Assert::IsFalse(constraint1.isCompletelyAssigned());
			var1.assignByValue(1.0);
			Assert::IsFalse(constraint1.isCompletelyAssigned());
			var2.assignByValue(2.5);
			Assert::IsFalse(constraint1.isCompletelyAssigned());
			var3.assignByValue(3.7);
			Assert::IsTrue(constraint1.isCompletelyAssigned());
		}

		TEST_METHOD(TestIsConsistent)
		{
			Assert::IsTrue(constraint1.isConsistent());
			var1.assignByValue(1.0);
			Assert::IsTrue(constraint1.isConsistent());
			var2.assignByValue(2.5);
			Assert::IsTrue(constraint1.isConsistent());
			var3.assignByValue(3.7);
			Assert::IsTrue(constraint1.isConsistent());
		}

		TEST_METHOD(TestIsSatisfied)
		{
			Assert::IsFalse(constraint1.isSatisfied());
			var1.assignByValue(1.0);
			Assert::IsFalse(constraint1.isSatisfied());
			var2.assignByValue(2.5);
			Assert::IsFalse(constraint1.isSatisfied());
			var3.assignByValue(3.7);
			Assert::IsTrue(constraint1.isSatisfied());
		}

		
		TEST_METHOD(TestEnforceUnaryConstraint)
		{
			std::unordered_set<double> expectedDomain{ 1.0, 2.5, 3.7, 4.2 };
			csp::Variable<double>& var = constraint7.getVariables().front();
			std::unordered_set<double> actualDomain(var.getDomain().cbegin(), var.getDomain().cend());
			Assert::IsTrue(actualDomain ==  expectedDomain);
		}
		
		
		TEST_METHOD(TestgetConsistentDomainValues)
		{
			var1.assignByValue(3.7);
			var2.assignByValue(3.7);
			const std::vector<double> consistentDomain = constraint2.getConsistentDomainValues(var3);
			Assert::IsTrue(consistentDomain.size() == 1);
			Assert::AreEqual(consistentDomain.front(), 3.7);
		}

		TEST_METHOD(TestToStringAndWriteToStreamOperator)
		{
			std::ostringstream outStringStream;
			outStringStream << constraint1;
			Assert::AreEqual(constraint1.toString(), outStringStream.str());
		}

		TEST_METHOD(TestEqualToOperator)
		{
			Assert::IsTrue(constraint1 == constraint1);
			csp::Constraint<double> constraint3{ vars, doubleAllEqual };
			Assert::IsFalse(constraint2 == constraint3);
		}

		TEST_METHOD(TestUncontainedVariableError)
		{
			csp::Variable<double> var4{ domain1 };
			Assert::ExpectException<csp::uncontained_variable_error<double>>([&]() -> void { auto consistentDom =
				constraint1.getConsistentDomainValues(var4); });
			try
			{
				auto consistentDom = constraint1.getConsistentDomainValues(var4);
			}
			catch (const csp::uncontained_variable_error<double>& excep)
			{
				Logger::WriteMessage("In TestUncontainedVariableError:");
				Logger::WriteMessage(excep.what());
			}
		}

		TEST_METHOD(TestDuplicateVariableError)
		{
			std::vector<std::reference_wrapper<csp::Variable<double>>> otherVars = vars;
			otherVars.emplace_back(var1);
			Assert::ExpectException<csp::duplicate_variable_error<double>>([&]() -> void { csp::Constraint<double> constraint3{ otherVars,
				[](const std::vector<double>& values) -> bool {return true; } }; });
			try
			{
				csp::Constraint<double> constraint3{ otherVars, [](const std::vector<double>& values) -> bool {return true; } };
			}
			catch (const csp::duplicate_variable_error<double>& excep)
			{
				Logger::WriteMessage("In TestDuplicateVariableError:");
				Logger::WriteMessage(excep.what());
			}
		}
	};
}