// MsCppUnitTestAdapter.h

#pragma once
#include <CPPUnitTestInvestigator/inc/CPPUnitTestInvestigator.h>

#include <string>
#include <memory>

using namespace System;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestPlatform::ObjectModel;

namespace MsCppUnitTestAdapter 
{

	std::string MarshalString(String ^ s) 
	{
		using namespace Runtime::InteropServices;
		const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
		std::string temp(chars);
		Marshal::FreeHGlobal(IntPtr((void*)chars));
		return temp;
	}

	public ref class TestInfo
	{
	public:
		property System::String ^ DisplayName;
		property System::String ^ FullyQualifiedName;
		property System::String ^ FileName;
		property int LineNumber;
		property System::String ^ ClassName;
	};
	
	public ref class VsTestAdapterModuleDiscoverer
	{
	public:
		static IEnumerable<TestInfo ^>^ DiscoverTests(System::String ^source);
	};

	public ref class VsTestAdapterExecutionContext
	{
		
		CppUnitTestInvestigator::TestExecutionContext *context_;

	public:
		VsTestAdapterExecutionContext(System::String ^sourceFile);
		~VsTestAdapterExecutionContext();

		void Execute(System::String ^methodName, TestResult ^res);
	};
}
