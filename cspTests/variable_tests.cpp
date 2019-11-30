#include "pch.h"
#include "CppUnitTest.h"
#include "variable.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace cspTests
{
	TEST_CLASS(VariableTests)
	{
	public:
		std::unordered_set<double> usetOriginalDomain{ 1.0, 2.5, 3.7, 4.2, 5.5, 6.6, 7.8, 8.8, 9.9, 10 };
		csp::Variable<double> var1{ usetOriginalDomain };


		TEST_METHOD_INITIALIZE(VariableSetUp)
		{
			var1.unassign();
		}

		TEST_METHOD(TestAssign)
		{
			Assert::IsFalse(var1.isAssigned());
			var1.assign(3.7);
			Assert::IsTrue(var1.isAssigned());
		}

		TEST_METHOD(TestUnassign)
		{
			var1.assign(3.7);
			var1.unassign();
			Assert::IsFalse(var1.isAssigned());
		}

		TEST_METHOD(TestGetValue)
		{
			var1.assign(3.7);
			Assert::AreEqual(var1.getValue(), 3.7);
		}

		TEST_METHOD(TestGetDomain)
		{
			const std::vector<double>& vecDomain = var1.getDomain();
			std::unordered_set<double> usetDomain(vecDomain.cbegin(), vecDomain.cend());
			Assert::IsTrue(usetDomain == usetOriginalDomain);
		}

		TEST_METHOD(TestToStringAndWriteToStreamOperator)
		{
			std::ostringstream outStringStream;
			outStringStream << var1;
			Assert::AreEqual(var1.toString(), outStringStream.str());
		}

		TEST_METHOD(TestEqualToOperator)
		{
			Assert::IsTrue(var1 == var1);
			csp::Variable<double> var2{ usetOriginalDomain };
			Assert::IsFalse(var1 == var2);
		}

		TEST_METHOD(TestUnassignedValueExtractionError)
		{
			Assert::ExpectException<csp::unassigned_value_extraction_error<double>>([&]() -> void { var1.getValue(); });
			try
			{
				var1.getValue();
			}
			catch (const csp::unassigned_value_extraction_error<double>& excep)
			{
				Logger::WriteMessage("In TestUnassignedValueExtractionError:");
				Logger::WriteMessage(excep.what());
			}
		}

		TEST_METHOD(TestOverAssignmentError)
		{
			var1.assign(3.7);
			Assert::ExpectException<csp::over_assignment_error<double>>([&]() -> void { var1.assign(2.5); });
			try
			{
				var1.assign(2.5);
			}
			catch (csp::over_assignment_error<double>& excep)
			{
				Logger::WriteMessage("In TestOverAssignmentError:");
				Logger::WriteMessage(excep.what());
			}
		}

		TEST_METHOD(TestUncontainedValueError)
		{
			Assert::ExpectException<csp::uncontained_value_error<double>>([&]() -> void { var1.assign(-1.5); });
			try
			{
				var1.assign(-1.5);
			}
			catch (csp::uncontained_value_error<double>& excep)
			{
				Logger::WriteMessage("In TestUncontainedValueError:");
				Logger::WriteMessage(excep.what());
			}
		}

		TEST_METHOD(TestRemoveFromDomain)
		{
			var1.removeFromDomain(6);
			const std::vector<double>& vecDomain = var1.getDomain();
			Assert::IsTrue(vecDomain.size() == 9);
		}
	};
}