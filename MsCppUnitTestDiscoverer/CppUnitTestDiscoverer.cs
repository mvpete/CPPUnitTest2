using Microsoft.VisualStudio.TestPlatform.ObjectModel.Adapter;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.VisualStudio.TestPlatform.ObjectModel.Logging;
using Microsoft.VisualStudio.TestPlatform.ObjectModel;

namespace MsCppUnitTestDiscoverer
{
    [FileExtension(".dll")]
    [DefaultExecutorUri(CppUnitTestDiscoverer.ExecutorUri)]
    [ExtensionUri(CppUnitTestDiscoverer.ExecutorUri)]
    public class CppUnitTestDiscoverer : ITestDiscoverer, ITestExecutor
    {
        public const String ExecutorUri = "executor://CppUnitTestDiscoverer";

        public void Cancel()
        {
            throw new NotImplementedException();
        }

        public void DiscoverTests(IEnumerable<string> sources, IDiscoveryContext discoveryContext, IMessageLogger logger, ITestCaseDiscoverySink discoverySink)
        {
            foreach (var source in sources)
            {
                foreach (var test in MsCppUnitTestAdapter.VsTestAdapterModuleDiscoverer.DiscoverTests(source))
                {
                    TestCase tc = new TestCase(test.FullyQualifiedName, new System.Uri(ExecutorUri), source);
                    tc.DisplayName = test.DisplayName;
                    tc.LineNumber = 3;
                    tc.CodeFilePath = "mycppunit.cpp";

                    discoverySink.SendTestCase(tc);
                }
            }
            
        }

        public void RunTests(IEnumerable<string> sources, IRunContext runContext, IFrameworkHandle frameworkHandle)
        {
            
        }

        public void RunTests(IEnumerable<TestCase> tests, IRunContext runContext, IFrameworkHandle frameworkHandle)
        {
            
        }
    }
}
