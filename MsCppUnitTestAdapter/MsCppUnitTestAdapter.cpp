// This is the main DLL file.

#include "stdafx.h"
#include "gcroot.h"

#include "MsCppUnitTestAdapter.h"

using namespace MsCppUnitTestAdapter;
using namespace CppUnitTestInvestigator;

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace System::Runtime::InteropServices;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// the help

String ^ GroupBySource(TestCase ^tc)
{
	return tc->Source;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VsTestAdapterModuleDiscoverer
System::Collections::Generic::IEnumerable<TestInfo ^>^ VsTestAdapterModuleDiscoverer::DiscoverTests(System::String ^source)
{
	try
	{
		TestModule tm(MarshalString(source));
		auto list = gcnew List<TestInfo ^>();

		// TODO: module attributes

		for (auto name : tm.GetModuleMethodNames())
		{
			TestInfo ^info = gcnew TestInfo();
			info->FullyQualifiedName = gcnew System::String(name.c_str());
			info->DisplayName = gcnew System::String(tm.GetMethodDisplayName(name).c_str());
			info->ClassName = gcnew System::String(tm.GetClassNameByMethodName(name).c_str());

			// TODO: method && class attributes

			auto metaData = tm.GetMethodInfoByName(name);

			info->LineNumber = metaData.lineNo;
			info->FileName = gcnew System::String(metaData.sourceFile);

			list->Add(info);
		}
		return list;
	}
	catch (...)
	{
		return gcnew List<TestInfo^>();
	}	
}

class ResultRecorder : public IExecutionCallback
{
	gcroot<TestResult ^> result_;

public:
	ResultRecorder(TestResult ^res)
		:result_(res)
	{
	}

	void OnComplete()
	{
		result_->EndTime = DateTimeOffset::Now;
		result_->Duration = result_->EndTime - result_->StartTime;
		result_->Outcome = TestOutcome::Passed;
	}

	void OnError(const char *msg) override
	{
		result_->EndTime = DateTimeOffset::Now;
		result_->Duration = result_->EndTime - result_->StartTime;
		result_->Outcome = TestOutcome::Failed;
		result_->ErrorMessage = gcnew System::String(msg);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VsTestAdapterExecutionContext
VsTestAdapterExecutionContext::VsTestAdapterExecutionContext(System::String ^sourcePath)
{
	context_ = new TestExecutionContext(MarshalString(sourcePath));
}

VsTestAdapterExecutionContext::~VsTestAdapterExecutionContext()
{
	delete context_;
}

void VsTestAdapterExecutionContext::Execute(System::String ^methodName, TestResult ^r)
{
	ResultRecorder cb(r);
	context_->Execute(MarshalString(methodName), &cb);
}