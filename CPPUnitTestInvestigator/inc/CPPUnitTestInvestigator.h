#ifndef __CPPUNITTEST_INVESTIGATOR_H__
#define __CPPUNITTEST_INVESTIGATOR_H__

#include "../src/PEUtils.h"
#include "CppUnitTest.h"

#ifdef CPPUNITTEST_EXPORTS
#define CPPUNITTEST_API __declspec(dllexport)
#else
#define CPPUNITTEST_API __declspec(dllimport)
#endif


#include <string>
#include <vector>
#include <memory>

namespace CppUnitTestInvestigator
{
	using ClassAttribute = std::pair<std::wstring, const void*>;
	using ModuleAttribute = std::pair<std::wstring, std::wstring>;
	using MethodAttribute = std::pair<std::wstring, const void*>;

	class CPPUNITTEST_API TestModuleInfo
	{
		PeUtils::PeExplorer load_;
		const std::string path_;

		Microsoft::VisualStudio::CppUnitTestFramework::TestDataVersion version_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> classMetadata_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::MethodMetadata> methodMetadata_;

		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ModuleAttributeMetadata> moduleAttributeMetadata_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::MethodAttributeMetadata> methodAttributeMetadata_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassAttributeMetadata> classAttributeMetadata_;

		void LoadData();

	public:

		TestModuleInfo(const std::string &modulePath);
		~TestModuleInfo();

		uint32_t GetVersion();
	
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> GetTestClassInfo() const;
		std::vector<std::string> GetModuleMethodNames() const;
		std::string GetMethodDisplayName(const std::string &methodName) const;
		std::vector<std::string> GetClassNames() const;

		std::string GetClassNameByMethodName(const std::string &methodName) const;
		std::string GetDecoratedMethodName(const std::string &methodName) const;
		bool GetDecoratedMethodName(const std::string &methodName, std::string &decoratedName) const;

		std::vector<std::string> GetMethodNames(const std::wstring &className) const;
		
		std::vector<MethodAttribute> GetMethodAttributes(const std::string &methodName) const;
		std::vector<ClassAttribute>  GetClassAttributes(const std::string &className) const;
		std::vector<ModuleAttribute> GetModuleAttributes() const;

		const ::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata & GetClassInfoByName(const std::string &className) const;
		const ::Microsoft::VisualStudio::CppUnitTestFramework::MethodMetadata & GetMethodInfoByName(const std::string &methodName) const;
		

		const std::string& Path() const;

	};

	class TestModule;

	class IReportException
	{
	public:
		virtual ~IReportException() = default;
		virtual void OnException(const wchar_t *message) = 0;
	};
	
	template<typename CallableT>
	class CallableExceptionHandler : public IReportException 
	{
		CallableT onException_;
	public:
		CallableExceptionHandler(CallableT &onException)
			:onException_(onException)
		{
		}
		virtual void OnException(const wchar_t *message) override
		{
			onException_(message);
		}

	};

	template <typename T>
	auto MakeExceptionHandler(T callable)
	{
		return CallableExceptionHandler<T>(callable);
	}

	class CPPUNITTEST_API TestClass_
	{
		TestModule &module_;

		const std::string className_;
		Microsoft::VisualStudio::CppUnitTestFramework::TestClassInfo *classInfo_;
		Microsoft::VisualStudio::CppUnitTestFramework::TestClassImpl *impl_;


	public:
		TestClass_(TestModule &module, const std::string &className);
		~TestClass_();

		void Initialize();
		void Cleanup();

		const std::string & Name() const;

		void Reset();

		void InvokeMethodSetup();
		void InvokeMethodCleanup();
		void InvokeMethod(const std::string &methodName);
	};

	class CPPUNITTEST_API TestModule
	{
		const TestModuleInfo moduleInfo_;
		HMODULE module_;
		IReportException *exceptionHandler_;

	private:
		void InitializeModule();
		void ModuleCleanup();

	private:
		static TestModule *Instance;

	public:
		TestModule(const std::string &source, IReportException *exceptionHandler);
		~TestModule();

		TestClass_* CreateClass(const std::string &className);
		TestClass_* CreateClassFromMethodName(const std::string &methodName);

		const TestModuleInfo & Info();
		template <typename FnT>
		FnT GetFunctionAddress(const char *name)
		{
			return GetProc<FnT>(module_, name);
		}

		LONG OnException(_EXCEPTION_POINTERS *exceptionInfo);
		
		static LONG __stdcall OnExceptionThunk(_EXCEPTION_POINTERS *ExceptionInfo);

	};
};


#endif // __CPPUNITTEST_INVESTIGATOR_H__