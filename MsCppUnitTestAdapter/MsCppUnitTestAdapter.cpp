// This is the main DLL file.

#include "stdafx.h"
#include "gcroot.h"

#include <stlx/inc/algorithm.h>


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
		TestModuleInfo tm(MarshalString(source));
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



class ResultRecorder
{
	bool isFailed_;
	gcroot<TestResult ^> result_;

public:
	ResultRecorder(TestResult ^res)
		:result_(res), isFailed_(false)
	{
	}

	virtual void OnStart()
	{
		result_->StartTime = DateTimeOffset::Now;
	}

	virtual void OnComplete()
	{
		if (isFailed_)
			return;
		result_->EndTime = DateTimeOffset::Now;
		result_->Duration = result_->EndTime - result_->StartTime;
		result_->Outcome = TestOutcome::Passed;
	}

	virtual void OnError(const char *msg)
	{
		OnError(gcnew System::String(msg));
	}

	virtual void OnError(const wchar_t *msg)
	{
		OnError(gcnew System::String(msg));
	}

	void OnError(System::String ^msg)
	{
		isFailed_ = true;
		result_->EndTime = DateTimeOffset::Now;
		result_->Duration = result_->EndTime - result_->StartTime;
		result_->Outcome = TestOutcome::Failed;
		result_->ErrorMessage = msg;
	}

	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResultReporterExceptionHandler

class ResultReporterExceptionHandler : public CppUnitTestInvestigator::IReportException
{
	ResultRecorder *result_;

public:
	ResultReporterExceptionHandler()
		:result_(nullptr)
	{
	}

	void Reset(ResultRecorder *result = nullptr)
	{
		result_ = result;
	}

	void OnException(const wchar_t *message)
	{
		if(result_ != nullptr)
			result_->OnError(System::String::Format("Assert Failed. {0}",gcnew System::String(message)));
	}
};


struct FindByClassName
{
	const std::string className;
	FindByClassName(const std::string &className)
		:className(className)
	{
	}

	bool operator()(const TestClass_ *ptr)
	{
		return ptr->Name() == className;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// VsTestAdapterExecutionContext - all C++ exceptions are caught and marshalled at this level.
VsTestAdapterExecutionContext::VsTestAdapterExecutionContext(System::String ^sourcePath)
{
	try
	{
		handler_ = new ResultReporterExceptionHandler();
		context_ = new TestModule(MarshalString(sourcePath), handler_);
		classes_ = new std::vector<TestClass_*>();

		// Module initialize
	}
	catch (const std::exception &e)
	{
		throw gcnew System::Exception(gcnew System::String(e.what()));
	}
	catch (...)
	{
		throw gcnew System::Exception("Unknown error loading test module.");
	}
}

VsTestAdapterExecutionContext::~VsTestAdapterExecutionContext()
{
	// module uninitialize

	delete handler_;
	delete context_;
	delete classes_;
}

void VsTestAdapterExecutionContext::LoadClass(System::String ^className)
{
	try
	{
		classes_->emplace_back(context_->CreateClass(MarshalString(className)));
		classes_->back()->Initialize();
	}
	catch (...)
	{
	}
}

void VsTestAdapterExecutionContext::UnloadClass(System::String ^className)
{
	try
	{
		auto name_s = MarshalString(className);
		auto class_ptr = stdx::find_if(*classes_, FindByClassName(name_s));

		class_ptr->Cleanup();
		delete class_ptr;
	}
	catch (...)
	{
	}
}

void VsTestAdapterExecutionContext::ExecuteMethod(System::String ^methodName, TestResult ^r)
{
	ResultRecorder cb(r); 
	static_cast<ResultReporterExceptionHandler*>(handler_)->Reset(&cb);
	try
	{			
		auto className = context_->Info().GetClassNameByMethodName(MarshalString(methodName));
		// load the test class, from the method name, this will load the class into the execution context, but it's name if it doesn't exist, it'll load it.
		TestClass_ *tc = nullptr;
		if (!stdx::find_if(*classes_, tc, FindByClassName(className)))
			tc = context_->CreateClass(className); // if you didn't call load, you won't get the class initialize.

		tc->Reset(); // this will reset the class i.e. create a new instance
		tc->InvokeMethodSetup();
		cb.OnStart();
		tc->InvokeMethod(MarshalString(methodName));
		cb.OnComplete();
		tc->InvokeMethodCleanup();
	}
	catch (const std::exception &e)
	{
		cb.OnError(System::String::Format("Uncaught C++ exception. {0}", gcnew System::String(e.what())));
	}
	catch (...)
	{
		cb.OnError("Unknown C++ Exception");
	}
	static_cast<ResultReporterExceptionHandler*>(handler_)->Reset();
	
}


