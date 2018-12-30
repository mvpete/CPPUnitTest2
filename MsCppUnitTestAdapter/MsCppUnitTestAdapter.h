// MsCppUnitTestAdapter.h

#pragma once
#include <CPPUnitTestInvestigator/inc/CPPUnitTestInvestigator.h>

#include <string>
#include <vector>
#include <memory>

using namespace System;
using namespace System::Collections::Generic;
using namespace Microsoft::VisualStudio::TestPlatform::ObjectModel;
using namespace Microsoft::VisualStudio::TestPlatform::ObjectModel::Adapter;

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

	public interface class IVsTestAdapterTestResult
	{
		void FailTest(System::String ^message);
	};
	
	public ref class VsTestAdapterExecutionContext
	{
		CppUnitTestInvestigator::IReportException *handler_;
		CppUnitTestInvestigator::TestModule *context_;
		std::vector<CppUnitTestInvestigator::TestClass_*> *classes_;
	public:
		VsTestAdapterExecutionContext(System::String ^sourceFile);
		~VsTestAdapterExecutionContext();

		void LoadClass(System::String ^className);
		void UnloadClass(System::String ^className);
		void ExecuteMethod(System::String ^methodName, TestResult ^res);
		
	};
}
