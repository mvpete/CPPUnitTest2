#ifndef __CPPUNITTEST_INVESTIGATOR_H__
#define __CPPUNITTEST_INVESTIGATOR_H__

#include "../src/PEUtils.h"
#include "CppUnitTest.h"

#include <string>
#include <vector>

#ifdef CPPUNITTEST_EXPORTS
#define CPPUNITTEST_API __declspec(dllexport)
#else
#define CPPUNITTEST_API __declspec(dllimport)
#endif

namespace CppUnitTestInvestigator
{
	class CPPUNITTEST_API TestModule
	{
		PeUtils::PeExplorer load_;
		

		Microsoft::VisualStudio::CppUnitTestFramework::TestDataVersion version_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> classMetadata_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::MethodMetadata> methodMetadata_;

		std::vector <::Microsoft::VisualStudio::CppUnitTestFramework::ModuleAttributeMetadata> moduleAttributeMetadata_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::MethodAttributeMetadata> methodAttributeMetadata_;
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassAttributeMetadata> classAttributeMetadata_;

		void LoadData();

	public:
		TestModule(const std::string &modulePath);
		~TestModule();
		uint32_t GetVersion();
		std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> GetTestClassInfo() const;
		std::vector<std::wstring> GetModuleMethodNames() const;
		std::vector<std::string> GetClassNames() const;
		std::vector<std::wstring> GetDecoratedClassNames() const;
		std::vector<std::wstring> GetDecoratedMethodNames() const;
		std::vector<std::wstring> GetMethodNames(const std::wstring &className) const;

		std::vector<std::pair<std::wstring, const void*>> GetMethodAttributes(const std::string &methodName) const;
		std::vector<std::pair<std::wstring, std::wstring>> GetModuleAttributes() const;
	};

};


#endif // __CPPUNITTEST_INVESTIGATOR_H__