#include <inc/CPPUnitTestInvestigator.h>

#include <algorithm>
#include <iterator>

using namespace CppUnitTestInvestigator;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// contains
template <typename IteratorT>
bool contains(const char *text, IteratorT begin, IteratorT end)
{
	IteratorT tmp = begin;
	while (*text != '\0' && tmp!=end)
	{
		if (*text == *tmp)
			++tmp;
		else if (tmp != begin)
			tmp = begin;
		
		++text;
	}
	if (tmp == end)
		return true;

	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TestModule
TestModule::TestModule(const std::string &path)
	:load_(path)
{
	LoadData();
}

TestModule::~TestModule()
{
}

uint32_t TestModule::GetVersion()
{
	return version_.version;
}

std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> TestModule::GetTestClassInfo() const
{
	return classMetadata_;
}

void TestModule::LoadData()
{
	auto data = load_.GetSection(".rdata"); // this is where static strings are kept
	auto testdata = load_.GetSection("testdata");

	auto version = load_.GetSection("testvers");
	version_ = *version.PointerToRawData<Microsoft::VisualStudio::CppUnitTestFramework::TestDataVersion*>();

	// get the first
	wchar_t **head = testdata.PointerToRawData<wchar_t **>();
	DWORD end = testdata.PointerToRawData() + testdata.SizeOfRawData();
	while ((DWORD)head < end)
	{
		if (*head == 0)
		{
			++head;
			continue;
		}

		wchar_t *tag = data.AdjustPointer(*head);
		if (wcscmp(tag, L"TestClassInfo") == 0)
		{
			ClassMetadata * mtd = reinterpret_cast<ClassMetadata*>(head);
			ClassMetadata adj = { tag,data.AdjustPointer(mtd->helpMethodName),data.AdjustPointer(mtd->helpMethodDecoratedName) };

			classMetadata_.emplace_back(adj);

			PeUtils::AdvancePointer(head, sizeof(ClassMetadata));
		}
		else if (wcscmp(tag, L"TestMethodInfo") == 0	       || 
				 wcscmp(tag, L"TestModuleInitializeInfo") == 0 ||
				 wcscmp(tag, L"TestModuleCleanupInfo") == 0    ||
				 wcscmp(tag, L"TestClassInitializeInfo") == 0  ||
				 wcscmp(tag, L"TestClassCleanupInfo") == 0     ||
				 wcscmp(tag, L"TestMethodInitializeInfo") == 0 ||
				 wcscmp(tag, L"TestMethodCleanupInfo") == 0      )
		{
			MethodMetadata * mtd = reinterpret_cast<MethodMetadata*>(head);

			const char *fnc = reinterpret_cast<const char*>(data.AdjustPointer(mtd->helpMethodName));
			const char *dfnc = reinterpret_cast<const char*>(data.AdjustPointer(mtd->helpMethodDecoratedName));
			const wchar_t *methodName = data.AdjustPointer(mtd->methodName);
			auto line = mtd->lineNo;
			const wchar_t *sourceFile = data.AdjustPointer(mtd->sourceFile);
			
			MethodMetadata adj = { tag, methodName, data.AdjustPointer(mtd->helpMethodName),data.AdjustPointer(mtd->helpMethodDecoratedName),sourceFile,line };

			methodMetadata_.emplace_back(adj);

			PeUtils::AdvancePointer(head, sizeof(MethodMetadata));
		}
		else if (wcscmp(tag, L"TestClassAttributeStart") == 0 || wcscmp(tag, L"TestClassAttribute") == 0 || wcscmp(tag, L"TestClassAttributeEnd") == 0)
		{
			ClassAttributeMetadata *mtd = reinterpret_cast<ClassAttributeMetadata*>(head);
			ClassAttributeMetadata adj = { tag, data.AdjustPointer(mtd->attributeName), data.AdjustPointer(mtd->attributeValue), ClassAttributeMetadata::CLASS_ATTRIBUTE };
			classAttributeMetadata_.emplace_back(adj);

			PeUtils::AdvancePointer(head, sizeof(ClassAttributeMetadata));
		}
		else if (wcscmp(tag, L"TestMethodAttributeStart") == 0 || wcscmp(tag, L"TestMethodAttribute") == 0 || wcscmp(tag, L"TestMethodAttributeEnd") == 0)
		{
			MethodAttributeMetadata *mtd = reinterpret_cast<MethodAttributeMetadata*>(head);
			const void *ptr = load_.FindPtr(mtd->attributeValue);
			MethodAttributeMetadata adj = { tag, data.AdjustPointer(mtd->attributeName), ptr!=nullptr?ptr:mtd->attributeValue, MethodAttributeMetadata::METHOD_ATTRIBUTE };
			methodAttributeMetadata_.emplace_back(adj);

			PeUtils::AdvancePointer(head, sizeof(MethodAttributeMetadata));
		}
		else if (wcscmp(tag, L"TestModuleAttributeStart") == 0 || wcscmp(tag, L"TestModuleAttribute") == 0 || wcscmp(tag, L"TestModuleAttributeEnd") == 0)
		{
			ModuleAttributeMetadata *mtd = reinterpret_cast<ModuleAttributeMetadata*>(head);
			ModuleAttributeMetadata adj = { tag, data.AdjustPointer(mtd->attributeName), data.AdjustPointer(mtd->attributeValue), ModuleAttributeMetadata::MODULE_ATTRIBUTE };
			moduleAttributeMetadata_.emplace_back(adj);

			PeUtils::AdvancePointer(head, sizeof(ModuleAttributeMetadata));
		}

	}
}

std::vector<std::pair<std::wstring, const void*>> TestModule::GetMethodAttributes(const std::string &methodName) const
{
	std::vector<std::pair<std::wstring, const void*>> attributes;
	bool found = false;
	for (auto attr : methodAttributeMetadata_)
	{
		const std::wstring tag(attr.tag); // to copy or not to copy, string view.
		if (tag.compare(L"TestMethodAttributeStart") == 0)
		{
			const char *fn = reinterpret_cast<const char*>(attr.attributeValue);
			if (contains(fn, methodName.begin(), methodName.end()))
				found = true;
		}
		else if (tag.compare(L"TestMethodAttributeEnd") == 0 && found)
			break;
		else if (found) 
			attributes.emplace_back(attr.attributeName, attr.attributeValue);
	}

	return attributes;
}


std::vector<std::pair<std::wstring, std::wstring>> TestModule::GetModuleAttributes() const
{
	std::vector<std::pair<std::wstring, std::wstring>> attributes;
	for (auto attr : moduleAttributeMetadata_)
	{
		std::wstring tag = attr.tag;
		std::wstring name = attr.attributeName != nullptr ? std::wstring(attr.attributeName) : std::wstring();
		std::wstring value = attr.attributeValue != nullptr ? std::wstring(attr.attributeValue) : std::wstring();
		if (tag.compare(L"TestModuleAttributeStart") == 0 || tag.compare(L"TestModuleAttributeEnd") == 0)
			continue;

		attributes.emplace_back(attr.attributeName, attr.attributeValue);
	}
	return attributes;
}