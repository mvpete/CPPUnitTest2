// This is the main DLL file.

#include "stdafx.h"

#include "MsCppUnitTestAdapter.h"

using namespace MsCppUnitTestAdapter;
using namespace CppUnitTestInvestigator;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VsTestAdapter
System::Collections::Generic::IEnumerable<TestInfo ^>^ VsTestAdapterModuleDiscoverer::DiscoverTests(System::String ^source)
{
	TestModule tm(MarshalString(source));
	auto list = gcnew List<TestInfo ^>();
	// TODO: should change this to get all the information about the tests;
	System::Int32 c = 0;
	for (auto name : tm.GetModuleMethodNames())
	{
		TestInfo ^info = gcnew TestInfo();
		info->FullyQualifiedName = gcnew System::String(name.c_str());
		info->DisplayName = gcnew System::String(tm.GetMethodDisplayName(name).c_str());
		list->Add(info);
		++c;
	}

	return list;
}