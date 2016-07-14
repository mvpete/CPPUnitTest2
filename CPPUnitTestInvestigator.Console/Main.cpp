
#include <CPPUnitTestInvestigator/inc/CPPUnitTestInvestigator.h>

using namespace CppUnitTestInvestigator;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef TestClassInfo* (*ClassInfoFn)();
typedef MemberMethodInfo* (*GetMemberMethodInfo)();

int main(int argc, const char **argv)
{
	throw std::exception();
	Assert::Fail();
	//TestModule md("CPPUnitTestInvestigator.Test.dll");
	//auto ver = md.GetTestClassInfo();

	//HMODULE h = ::LoadLibrary("C:\\workspace\\CPPUnitTestInvestigator\\Debug\\CPPUnitTestInvestigator.Test.dll");
	//if (h == 0)
	//	auto err = ::GetLastError();

	//ClassInfoFn fn = reinterpret_cast<ClassInfoFn>(::GetProcAddress(h, reinterpret_cast<const char*>(ver[1].helpMethodDecoratedName)));

	//auto res = fn();

	//// This thing needs to run the Module load function, after the module loads
	//// Then it needs to load the class and call the class initialization function
	//// Then before each run call the method initialization function
	//// Run the function
	//// Call the method clean up
	//// Call the class clean up

	//auto cls = res->pNewMethod();


	//GetMemberMethodInfo gmmd = reinterpret_cast<GetMemberMethodInfo>(::GetProcAddress(h,md.GetDecoratedMethodName("DummyAssert").c_str()));


	//auto mtd = gmmd();

	//cls->__Invoke(mtd->method.pVoidMethod);

	//::FreeLibrary(h);

	//int i = 9;
}