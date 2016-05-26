#include <inc/CPPUnitTestInvestigator.h>
#include <string_span.h>

#include <algorithm>
#include <iterator>
#include <regex>

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

template <typename IteratorT>
int compare(IteratorT begin, IteratorT end, IteratorT begin2, IteratorT end2)
{
	auto s1 = end - begin;
	auto s2 = end2 - begin2;

	if (s1 < s2)
		return 1;
	if (s1 > s2)
		return -1;

	while (begin != end)
	{
		if (*begin > *begin2)
			return -1;
		else if (*begin < *begin2)
			return 1;

		++begin;
		++begin2;
	}
	return 0;
}

template <typename ContainerT>
int compare(ContainerT &rhs, ContainerT &lhs)
{
	return compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}


bool ParseClassNameFromInnerTemplate(const std::string &fullName, std::string &className)
{
	// how do I parse the class name??
	// extract the <contents> then pull the class as the last :: 
	std::regex rn("<(.+)>");
	std::smatch match;
	if (std::regex_search(fullName, match, rn))
	{
		std::string clsname = match[1].str();
		size_t pos = clsname.rfind("::");
		if (pos != std::string::npos && pos + 2 < clsname.size())
		{
			className = clsname.substr(pos + 2);
			return true;
		}
	}
	return false;
}

bool ParseClassNameFromMethodName(const std::string &fullName, std::string &className)
{
	auto fo = fullName.rfind("::");
	if (fo == std::string::npos)
		return false;
	auto so = fullName.rfind("::", fo-1);
	if (so == std::string::npos)
		return false;
	className = fullName.substr(so+2, fo-so-2);
	return true;
	
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
		else if (wcscmp(tag, L"TestMethodInfo") == 0 ||
			wcscmp(tag, L"TestModuleInitializeInfo") == 0 ||
			wcscmp(tag, L"TestModuleCleanupInfo") == 0 ||
			wcscmp(tag, L"TestClassInitializeInfo") == 0 ||
			wcscmp(tag, L"TestClassCleanupInfo") == 0 ||
			wcscmp(tag, L"TestMethodInitializeInfo") == 0 ||
			wcscmp(tag, L"TestMethodCleanupInfo") == 0)
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
			MethodAttributeMetadata adj = { tag, data.AdjustPointer(mtd->attributeName), ptr != nullptr ? ptr : mtd->attributeValue, MethodAttributeMetadata::METHOD_ATTRIBUTE };
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



uint32_t TestModule::GetVersion()
{
	return version_.version;
}

std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> TestModule::GetTestClassInfo() const
{
	return classMetadata_;
}

std::vector<std::wstring> TestModule::GetModuleMethodNames() const
{
	std::vector<std::wstring> methods;
	std::transform(methodMetadata_.begin(), methodMetadata_.end(), std::back_inserter(methods), [](const MethodMetadata &mtd)
	{
		if (mtd.methodName == nullptr)
			return std::wstring();
		return std::wstring(mtd.methodName);
	});
	return methods;
}

std::vector<MethodAttribute> TestModule::GetMethodAttributes(const std::string &methodName) const
{
	std::vector<MethodAttribute> attributes;
	bool found = false;
	for (auto attr : methodAttributeMetadata_)
	{
		gsl::cwstring_span<> tag = gsl::ensure_z(attr.tag,30);
		gsl::cwstring_span<> start = gsl::ensure_z(L"TestMethodAttributeStart");
		gsl::cwstring_span<> end = gsl::ensure_z(L"TestMethodAttributeEnd");

		if (compare(tag, start) == 0)
		{
			const char *fn = reinterpret_cast<const char*>(attr.attributeValue);
			if (contains(fn, methodName.begin(), methodName.end()))
				found = true;
		}
		else if (compare(tag,end) == 0 && found)
			break;
		else if (found) 
			attributes.emplace_back(attr.attributeName, attr.attributeValue);
	}

	return attributes;
}

std::vector<ClassAttribute> TestModule::GetClassAttributes(const std::string &className) const
{
	bool found = false;
	std::vector<ClassAttribute> attributes;
	for (auto attr : classAttributeMetadata_)
	{
		gsl::cwstring_span<> tag = gsl::ensure_z(attr.tag);
		gsl::cwstring_span<> start = gsl::ensure_z(L"TestClassAttributeStart");
		gsl::cwstring_span<> end = gsl::ensure_z(L"TestClassAttributeEnd");

		if (compare(tag, start) == 0)
		{
			gsl::cstring_span<> fullName = gsl::ensure_z(reinterpret_cast<const char*>(attr.attributeValue));
			std::string fullName_s(fullName.begin(), fullName.end());
			std::string parsedName;
			if (ParseClassNameFromMethodName(fullName_s, parsedName) && parsedName.compare(className) == 0)
			{
				found = true;
			}
		}
		else if (compare(tag, end) == 0 && found)
		{
			break;
		}
		else if(found)
			attributes.emplace_back(attr.attributeName, attr.attributeValue);
	}
	return attributes;
}

std::vector<ModuleAttribute> TestModule::GetModuleAttributes() const
{
	std::vector<ModuleAttribute> attributes;
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

std::vector<std::string> TestModule::GetClassNames() const
{
	std::vector<std::string> classes;
	for (auto cls : classMetadata_)
	{
		std::string classname;
		if (ParseClassNameFromInnerTemplate(reinterpret_cast<const char*>(cls.helpMethodName), classname))
		{
			classes.emplace_back(classname);
		}

		
	}
	return classes;
}