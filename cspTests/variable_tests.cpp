#include "pch.h"
#include "CppUnitTest.h"
#include <csp.h>


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

		TEST_METHOD(TestCopyCtor)
		{
			csp::Variable<double> var2{ usetOriginalDomain };
			var2.assignByValue(1.0);
			csp::Variable<double> var4{ var2 };
			Assert::IsTrue(var2.getValue() == var4.getValue());
			std::unordered_set<double> var2Domain{ var2.getDomain().cbegin(), var2.getDomain().cend() };
			std::unordered_set<double> var4Domain{ var4.getDomain().cbegin(), var4.getDomain().cend() };
			Assert::IsTrue(var2Domain == var4Domain);
		}

		TEST_METHOD(TestCopyAssignmentOperator)
		{
			csp::Variable<double> var2{ usetOriginalDomain };
			var2.assignByValue(1.0);
			csp::Variable<double> var3{ {1, 2, 3, 4, 5} };
			var3.assignByValue(5);
			var3 = var2;
			Assert::IsTrue(var3.getDomain().size() == 10);
			Assert::IsTrue(var3.getValue() == 1.0);
		}

		TEST_METHOD(TestMoveCtor)
		{
			csp::Variable<double> var7{ usetOriginalDomain };
			var7.assignByValue(1.0);
			csp::Variable<double> var8{ std::move(var7) };
			Assert::IsTrue(var7.getDomain().empty());
			Assert::IsTrue(var8.getDomain().size() == 10);
			Assert::IsTrue(var8.getValue() == 1.0);
		}

		TEST_METHOD(TestMoveAssignmentOperator)
		{
			csp::Variable<double> var2{ usetOriginalDomain };
			var2.assignByValue(2.5);
			csp::Variable<double> var3{ {1, 2, 3, 4, 5} };
			var3.assignByValue(5);
			var2 = std::move(var3);
			Assert::IsTrue(var2.getDomain().size() == 5);
			Assert::IsTrue(var2.getValue() == 5);
			Assert::IsTrue(var3.getDomain().size() == 10);
		}
		TEST_METHOD(TestAssign)
		{
			Assert::IsFalse(var1.isAssigned());
			var1.assignByValue(3.7);
			Assert::IsTrue(var1.isAssigned());
		}

		TEST_METHOD(TestUnassign)
		{
			var1.assignByValue(3.7);
			var1.unassign();
			Assert::IsFalse(var1.isAssigned());
		}

		TEST_METHOD(TestGetValue)
		{
			var1.assignByValue(3.7);
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

		TEST_METHOD(TestRemoveFromDomainByIdx)
		{
			csp::Variable<double> var2{ usetOriginalDomain };
			const std::vector<double>& domain = var2.getDomain();
			size_t idxOfValToDelete = 3;
			double valToDelete = domain[idxOfValToDelete];
			var2.removeFromDomainByIdx(idxOfValToDelete);
			Assert::IsTrue(std::find(domain.cbegin(), domain.cend(), valToDelete) == domain.cend());
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
			var1.assignByValue(3.7);
			Assert::ExpectException<csp::over_assignment_error<double>>([&]() -> void { var1.assignByValue(2.5); });
			try
			{
				var1.assignByValue(2.5);
			}
			catch (csp::over_assignment_error<double>& excep)
			{
				Logger::WriteMessage("In TestOverAssignmentError:");
				Logger::WriteMessage(excep.what());
			}
		}

		TEST_METHOD(TestUncontainedValueError)
		{
			Assert::ExpectException<csp::uncontained_value_error<double>>([&]() -> void { var1.assignByValue(-1.5); });
			try
			{
				var1.assignByValue(-1.5);
			}
			catch (csp::uncontained_value_error<double>& excep)
			{
				Logger::WriteMessage("In TestUncontainedValueError:");
				Logger::WriteMessage(excep.what());
			}
		}

		TEST_METHOD(TestRemoveFromDomain)
		{
			var1.removeFromDomainByIdx(6);
			const std::vector<double>& vecDomain = var1.getDomain();
			Assert::IsTrue(vecDomain.size() == 9);
		}
	};
}