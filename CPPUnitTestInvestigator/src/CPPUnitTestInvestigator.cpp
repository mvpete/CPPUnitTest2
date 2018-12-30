#include <inc/CPPUnitTestInvestigator.h>
#include <Exception>
#include <string_span.h>

#include <algorithm>
#include <map>
#include <iterator>
#include <regex>
#include <codecvt>

#include <stlx/inc/algorithm.h>

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

template <typename IteratorT, typename IteratorT2>
int compare(IteratorT begin, IteratorT end, IteratorT2 begin2, IteratorT2 end2)
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

using convert_type = std::codecvt_utf8<wchar_t>;
std::string to_string(const std::wstring &str)
{
	static std::wstring_convert<convert_type, wchar_t> converter;
	return converter.to_bytes(str);
}

template <typename T>
std::string from_char_ptr(const T* ptr)
{
	if (!ptr)
		return std::string();

	return std::string(reinterpret_cast<const char*>(ptr));
}


std::wstring to_wstring(const std::string &str)
{
	static std::wstring_convert<convert_type, wchar_t> converter;
	return converter.from_bytes(str);
}

template <typename FnT>
FnT GetProc(HMODULE h, const char *fn)
{
	return reinterpret_cast<FnT>(::GetProcAddress(h, fn));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// helper function ptrs

typedef GlobalMethodInfo* (*GlobalMethodInfoLoadFn)();
typedef MemberMethodInfo* (*MembMethodInfoLoadFn)();
typedef TestClassInfo* (*GetClassInfoFn)();


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TestModuleInfo
TestModuleInfo::TestModuleInfo(const std::string &path)
	:load_(path), path_(path)
{
	LoadData();
}

TestModuleInfo::~TestModuleInfo() = default;

void TestModuleInfo::LoadData()
{
	auto version = load_.GetSection("testvers");
	if (!version.IsValid())
		throw std::runtime_error("invalid DLL");

	version_ = *version.PointerToRawData<Microsoft::VisualStudio::CppUnitTestFramework::TestDataVersion*>();
	
	auto data = load_.GetSection(".rdata"); // this is where static strings are kept
	auto testdata = load_.GetSection("testdata");
	
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

uint32_t TestModuleInfo::GetVersion()
{
	return version_.version;
}

std::vector<::Microsoft::VisualStudio::CppUnitTestFramework::ClassMetadata> TestModuleInfo::GetTestClassInfo() const
{
	return classMetadata_;
}

std::vector<std::string> TestModuleInfo::GetModuleMethodNames() const
{
	auto cpy = methodMetadata_;

	stdx::erase_if(cpy, [](const MethodMetadata &mtd)
	{
		return stdx::contains(std::wstring(mtd.tag), std::wstring(L"InitializeInfo")) || stdx::contains(std::wstring(mtd.tag), std::wstring(L"CleanupInfo"));
	});

	std::vector<std::string> methods;
	std::transform(cpy.begin(), cpy.end(), std::back_inserter(methods), [](const MethodMetadata &mtd)
	{

		if (mtd.methodName == nullptr)
			return std::string();
		return to_string(mtd.methodName);
	});
	return methods;
}

std::string TestModuleInfo::GetMethodDisplayName(const std::string &methodName) const
{
	auto attrs = GetMethodAttributes(methodName);
	auto name = std::find_if(attrs.begin(), attrs.end(), [](const MethodAttribute & attr)
	{
		return attr.first.compare(L"TestName") == 0;
	});

	if (name != attrs.end())
		return std::string(from_char_ptr(name->second));
	else
		return methodName;
}

std::string TestModuleInfo::GetDecoratedMethodName(const std::string &methodName) const
{
	auto info = GetMethodInfoByName(methodName);

	return std::string(reinterpret_cast<const char*>(info.helpMethodDecoratedName));
}

bool TestModuleInfo::GetDecoratedMethodName(const std::string &methodName, std::string &decoratedName) const
{
	// TODO: Restructure this so that I don't need the T/C
	try
	{
		auto info = GetMethodInfoByName(methodName);
		decoratedName = reinterpret_cast<const char*>(info.helpMethodDecoratedName);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

std::vector<MethodAttribute> TestModuleInfo::GetMethodAttributes(const std::string &methodName) const
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

std::vector<ClassAttribute> TestModuleInfo::GetClassAttributes(const std::string &className) const
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

std::vector<ModuleAttribute> TestModuleInfo::GetModuleAttributes() const
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

std::vector<std::string> TestModuleInfo::GetClassNames() const
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

std::string TestModuleInfo::GetClassNameByMethodName(const std::string &methodName) const
{
	auto info = GetMethodInfoByName(methodName);

	std::string className;
	if (!ParseClassNameFromMethodName(from_char_ptr(info.helpMethodName), className))
		throw std::runtime_error("failed to find class by method name");

	return className;
}

const ClassMetadata & TestModuleInfo::GetClassInfoByName(const std::string &className) const
{
	auto metadata = std::find_if(classMetadata_.begin(), classMetadata_.end(), [&className](const ClassMetadata &cls)
	{
		std::string parsed;
		if (ParseClassNameFromInnerTemplate(from_char_ptr(cls.helpMethodName), parsed))
		{
			return parsed.compare(className) == 0;
		}
		else
			return false;
	});

	if (metadata != classMetadata_.end())
		return *metadata;

	throw std::runtime_error("failed to find class");
}

const MethodMetadata & TestModuleInfo::GetMethodInfoByName(const std::string &methodName) const
{
	auto i = std::find_if(methodMetadata_.begin(), methodMetadata_.end(), [&methodName](const MethodMetadata &mtd)
	{
		auto nr = gsl::ensure_z(mtd.methodName);

		return compare(nr.begin(), nr.end(), methodName.begin(), methodName.end()) == 0;
	});

	if (i == methodMetadata_.end())
		throw std::runtime_error("failed to find method");

	return *i;
}

const std::string& TestModuleInfo::Path() const
{
	return path_;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TestModule -- will load the test DLL and initialize it, on dtor run the cleanup
TestModule::TestModule(const std::string &source, IReportException *exceptionHandler)
	:moduleInfo_(source), exceptionHandler_(exceptionHandler)
{
	// This is where the framework dll sits
	::SetDllDirectoryA("C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\Common7\\IDE\\CommonExtensions\\Microsoft\\TestWindow\\Extensions\\CppUnitFramework");
	Instance = this;
	auto h = AddVectoredExceptionHandler(1, &OnExceptionThunk);
	module_ = ::LoadLibrary(moduleInfo_.Path().c_str());
	if (module_ == 0)
	{
		throw std::runtime_error("failed to setup test execution context"); // hehe. fttfo.
	}
}

TestModule::~TestModule()
{
	::FreeLibrary(module_);
}

TestModule * TestModule::Instance = nullptr;

void TestModule::InitializeModule()
{
	std::string initDecorated;
	if (!moduleInfo_.GetDecoratedMethodName("TestModuleInitializeInfo", initDecorated))
		return;

	auto moduleInit = GetProc<GlobalMethodInfoLoadFn>(module_, initDecorated.c_str());

	auto helper = moduleInit();

	// run the module load function
	helper->pVoidFunc();
}

void TestModule::ModuleCleanup()
{
	std::string initDecorated;
	if (!moduleInfo_.GetDecoratedMethodName("TestModuleCleanupInfo", initDecorated))
		return;

	auto moduleInit = GetProc<GlobalMethodInfoLoadFn>(module_, initDecorated.c_str());

	auto helper = moduleInit();

	// run the module load function
	helper->pVoidFunc();
}

TestClass_* TestModule::CreateClass(const std::string &className)
{
	// this will create a new TestClass_ and call it's initialize, since that is global it's okay.
	return new TestClass_(*this, className);

}

TestClass_* TestModule::CreateClassFromMethodName(const std::string &methodName)
{
	auto className = moduleInfo_.GetClassNameByMethodName(methodName);
	return CreateClass(className);
}

const TestModuleInfo& TestModule::Info()
{
	return moduleInfo_;
}

LONG TestModule::OnException(_EXCEPTION_POINTERS *exceptionInfo)
{
	//  if the code is the MS Unit test exception
	if (exceptionInfo->ExceptionRecord->ExceptionCode == 0xe3530001)
	{
		exceptionHandler_->OnException(reinterpret_cast<const wchar_t*>(exceptionInfo->ExceptionRecord->ExceptionInformation[0]));
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
		return EXCEPTION_CONTINUE_SEARCH;
}

LONG TestModule::OnExceptionThunk(_EXCEPTION_POINTERS *ExceptionInfo)
{
	if (Instance == nullptr)
		return 0;

	return Instance->OnException(ExceptionInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TestClass_

TestClass_::TestClass_(TestModule &module, const std::string &className)
	:module_(module), impl_(nullptr), className_(className)
{
	auto cInfoHelp = module_.Info().GetClassInfoByName(className);
	auto cInfoFn = module_.GetFunctionAddress<GetClassInfoFn>(reinterpret_cast<const char*>(cInfoHelp.helpMethodDecoratedName));
	auto cInfo = cInfoFn();
	if (cInfo != classInfo_)
	{
		classInfo_ = cInfo;
	}

	
}

TestClass_::~TestClass_()
{
	if(impl_)
		classInfo_->pDeleteMethod(impl_);
}

void TestClass_::Reset()
{
	if (impl_)
		classInfo_->pDeleteMethod(impl_);

	impl_ = classInfo_->pNewMethod();
}

const std::string& TestClass_::Name() const
{
	return className_;
}

void TestClass_::Initialize()
{
	// load the static class initializer here.
}

void TestClass_::Cleanup()
{
	// load the static class initilizer here
}

void TestClass_::InvokeMethodSetup()
{
	// load the test class method initialzer
}

void TestClass_::InvokeMethodCleanup()
{
	// load the test class teardown
}

void TestClass_::InvokeMethod(const std::string &methodName)
{
	auto mInfoFn = module_.GetFunctionAddress<MembMethodInfoLoadFn>(module_.Info().GetDecoratedMethodName(methodName).c_str());
	auto mInfo = mInfoFn();
		
	impl_->__Invoke(mInfo->method.pVoidMethod); // hmmm... 

}
