#include "stdafx.h"
#include "CppUnitTest.h"

#include "CPPUnitTestInvestigator.h"


#include <Windows.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

std::string IntrospecDllPath()
{
	HMODULE hm;

	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPWSTR)&IntrospecDllPath, &hm);

	char buffer[512];
	::GetModuleFileNameA(hm, buffer, 512);
	return std::string(buffer);
}

using namespace CppUnitTestInvestigator;

template<typename ContainerT>
class IteratorToString 
{
	const ContainerT &container_;
	typename ContainerT::const_iterator iterator_;
public:
	IteratorToString(const ContainerT &cont, typename ContainerT::const_iterator &it)
		:container_(cont), iterator_(it)
	{
	}

	virtual std::wstring ToString() const
	{
		if (iterator_ == std::end(container_))
			return std::wstring(L"End of Collection");
		return Microsoft::VisualStudio::CppUnitTestFramework::ToString(*iterator_);
	}

	bool operator==(const IteratorToString &rhs) const
	{
		return iterator_ == rhs.iterator_;
	}

	bool operator!=(const IteratorToString &rhs) const
	{
		return !operator==(rhs);
	}
};


template <typename T>
const IteratorToString<T> Wrap(const T& cnt, typename T::const_iterator &it)
{
	return IteratorToString<T>(cnt, it);
}

template<> inline std::wstring Microsoft::VisualStudio::CppUnitTestFramework::ToString<IteratorToString<std::vector<std::string>>>(const IteratorToString<std::vector<std::string>> &tsw)
{
	return tsw.ToString();
}


/// Woah. These tests are so.... meta.

namespace CPPUnitTestInvestigatorTest
{		
	BEGIN_TEST_MODULE_ATTRIBUTE()
		TEST_MODULE_ATTRIBUTE(L"ModuleAttribute1", L"ModuleAtribute1Value")
		TEST_MODULE_ATTRIBUTE(L"ModuleAttribute2", L"ModuleAtribute2Value")
		TEST_MODULE_ATTRIBUTE(L"ModuleAttribute3", L"ModuleAtribute3Value")
		TEST_MODULE_ATTRIBUTE(L"ModuleAttribute4", L"ModuleAtribute4Value")
		TEST_MODULE_ATTRIBUTE(L"ModuleAttribute5", L"ModuleAtribute5Value")
	END_TEST_MODULE_ATTRIBUTE()

	namespace nested
	{
		
		TEST_CLASS(DummyClass)
		{
			BEGIN_TEST_CLASS_ATTRIBUTE()
				TEST_CLASS_ATTRIBUTE(L"TestClassAttribute1", L"TestClassAttributeValue")
				TEST_CLASS_ATTRIBUTE(L"TestClassAttribute2", L"TestClassAttributeValue")
				TEST_CLASS_ATTRIBUTE(L"TestClassAttribute3", L"TestClassAttributeValue")
			END_TEST_CLASS_ATTRIBUTE()

				
			TEST_METHOD(DummyAssert)
			{
				Assert::IsTrue(true);
			}
		};
	}


	TEST_CLASS(CPPUnitTestIntrospection)
	{
	public:

		BEGIN_TEST_CLASS_ATTRIBUTE()
			TEST_CLASS_ATTRIBUTE(L"CPPUintTestAttribute", "Test")
			END_TEST_CLASS_ATTRIBUTE()

			// This test is testing that the currently loaded test DLL is the same as the 
			TEST_METHOD(TestVersionMatch)
		{
			TestModule tm(IntrospecDllPath());
			Assert::IsTrue(tm.GetVersion() == __CPPUNITTEST_VERSION__);
		}

		TEST_METHOD(TestGetModuleMethodName)
		{
			TestModule tm(IntrospecDllPath());
			auto methods = tm.GetModuleMethodNames();

			Assert::AreNotEqual(Wrap(methods, std::end(methods)), Wrap(methods, std::find(std::begin(methods), std::end(methods), std::string("TestGetModuleMethodName"))), L"TestMethod not found");
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(TestGetMethodAttributes)
			TEST_METHOD_ATTRIBUTE(L"TestAttribute", "TestAttributeValue")
			TEST_METHOD_ATTRIBUTE(L"TestAttributeIntValue", reinterpret_cast<const void*>(5))
			END_TEST_METHOD_ATTRIBUTE()

			TEST_METHOD(TestGetMethodAttributes)
		{
			TestModule tm(IntrospecDllPath());
			auto attributes = tm.GetMethodAttributes("TestGetMethodAttributes");

			Assert::AreEqual(size_t(2), attributes.size());
			Assert::AreEqual(std::string("TestAttributeValue"), std::string(reinterpret_cast<const char*>(attributes[0].second)));
			Assert::AreEqual(5, reinterpret_cast<int>(attributes[1].second));
		}

		TEST_METHOD(TestGetModuleAttributes)
		{
			TestModule tm(IntrospecDllPath());
			auto attributes = tm.GetModuleAttributes();

			Assert::IsTrue(attributes.size() == 5);
		}

		TEST_METHOD(TestGetClassNames)
		{
			TestModule tm(IntrospecDllPath());
			auto classes = tm.GetClassNames();

			Assert::AreEqual(size_t(2), classes.size());
			Assert::AreEqual(std::string("CPPUnitTestIntrospection"), classes[0]);
			Assert::AreEqual(std::string("DummyClass"), classes[1]);
		}

		TEST_METHOD(TestGetDummyClassAttributes)
		{
			TestModule tm(IntrospecDllPath());

			auto attributes = tm.GetClassAttributes("DummyClass");
			Assert::AreEqual(size_t(3), attributes.size());
		}

		TEST_METHOD(TestGetClassInfo)
		{
			TestModule tm(IntrospecDllPath());

			auto attributes = tm.GetTestClassInfo();
			Assert::AreEqual(size_t(2), attributes.size());
		}

		BEGIN_TEST_METHOD_ATTRIBUTE(TestOverrideTestName)
			TEST_METHOD_ATTRIBUTE(L"TestName", "My Override Test Name")
		END_TEST_METHOD_ATTRIBUTE()

		TEST_METHOD(TestOverrideTestName)
		{
			TestModule tm(IntrospecDllPath());

			std::string displayName = tm.GetMethodDisplayName("TestOverrideTestName");
			Assert::AreEqual(std::string("My Override Test Name"), displayName);
		}
		
		TEST_METHOD(TestGetTestClassByMethod)
		{
			TestModule tm(IntrospecDllPath());

			std::string myClass = tm.GetClassNameByMethodName("TestGetTestClassByMethod");
			
			Assert::AreEqual(std::string("CPPUnitTestIntrospection"), myClass, L"class name mismatch");
		}

		TEST_METHOD(ThisTestWillFail)
		{
			//Logger::WriteMessage(L"Logged");
			throw std::runtime_error("Test Failed with a KNOWN C++ Exception");
		}

	};
}