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
	using ClassAttribute = std::pair<std::wstring, const void*>;
	using ModuleAttribute = std::pair<std::wstring, std::wstring>;
	using MethodAttribute = std::pair<std::wstring, const void*>;

	class CPPUNITTEST_API TestModule
	{
		PeUtils::PeExplorer load_;
		const std::string path_;

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
		std::vector<std::string> GetModuleMethodNames() const;
		std::string GetMethodDisplayName(const std::string &methodName) const;
		std::vector<std::string> GetClassNames() const;

		std::string GetClassNameByMethodName(const std::string &methodName) const;
		std::string GetDecoratedMethodName(const std::string &methodName) const;

		std::vector<std::string> GetMethodNames(const std::wstring &className) const;

		std::vector<MethodAttribute> GetMethodAttributes(const std::string &methodName) const;
		std::vector<ClassAttribute>  GetClassAttributes(const std::string &className) const;
		std::vector<ModuleAttribute> GetModuleAttributes() const;

		const ::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata & GetClassInfoByName(const std::string &className) const;
		const ::Microsoft::VisualStudio::CppUnitTestFramework::MethodMetadata & GetMethodInfoByName(const std::string &methodName) const;

		void Execute(const std::vector<std::string> &methods);
	};

};


#endif // __CPPUNITTEST_INVESTIGATOR_H__