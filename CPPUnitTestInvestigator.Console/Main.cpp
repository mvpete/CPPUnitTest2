
#include <CPPUnitTestInvestigator/inc/CPPUnitTestInvestigator.h>

using namespace CppUnitTestInvestigator;


int main(int argc, const char **argv)
{
	TestModule md("CPPUnitTestInvestigator.Test.dll");
	auto ver = md.IsCurrentVersion();

	int i = 9;
}