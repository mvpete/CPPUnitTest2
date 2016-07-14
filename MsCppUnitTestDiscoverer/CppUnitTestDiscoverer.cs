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
            foreach (TestCase tc in DiscoverTests(sources))
                discoverySink.SendTestCase(tc);
        }

        public void RunTests(IEnumerable<string> sources, IRunContext runContext, IFrameworkHandle frameworkHandle)
        {
            ExecuteTests(DiscoverTests(sources), frameworkHandle);
        }

        public void RunTests(IEnumerable<TestCase> tests, IRunContext runContext, IFrameworkHandle frameworkHandle)
        {
            ExecuteTests(tests, frameworkHandle);
        }

        private List<TestCase> DiscoverTests(IEnumerable<String> sources)
        {
            List<TestCase> cases = new List<TestCase>();

            foreach (var source in sources)
            {
                foreach (var test in MsCppUnitTestAdapter.VsTestAdapterModuleDiscoverer.DiscoverTests(source))
                {
                    TestCase tc = new TestCase(test.FullyQualifiedName, new System.Uri(ExecutorUri), source);
                    tc.DisplayName = test.DisplayName;
                    tc.LineNumber = test.LineNumber;
                    tc.CodeFilePath = test.FileName;
                    var tp = TestProperty.Find("ClassName")  ?? TestProperty.Register("ClassName", "ClassName", typeof(string), typeof(TestCase));
                    tc.SetPropertyValue(tp, test.ClassName);
                    cases.Add(tc);
                }
            }
            return cases;
        }

        private void ExecuteTests(IEnumerable<TestCase> tests, IFrameworkHandle frameworkHandle)
        {
            // sort the tests by source.
            var ordered = tests.GroupBy(tc => tc.Source);

            TestResult res = null;
            foreach (var srcList in ordered)
            {
                try
                {
                    // Setup Execution Context
                    var ectx = new MsCppUnitTestAdapter.VsTestAdapterExecutionContext(srcList.Key);
                    // Execute each test

                    var classes = srcList.GroupBy(tc => tc.GetPropertyValue<String>(TestProperty.Find("ClassName"), ""));

                    foreach (var testClassList in classes)
                    {
                        foreach (var test in testClassList)
                        {
                            res = new TestResult(test);
                            res.StartTime = DateTimeOffset.Now;

                            frameworkHandle.RecordStart(test);

                            ectx.Execute(test.FullyQualifiedName, res);

                            frameworkHandle.RecordResult(res);
                        }
                    }
                }
                catch (Exception e)
                {
                    res.EndTime = DateTimeOffset.Now;
                    res.Duration = res.EndTime - res.StartTime;
                    res.Outcome = TestOutcome.Failed;
                    res.ErrorMessage = e.Message;
                    res.ErrorStackTrace = e.StackTrace;
                    frameworkHandle.RecordResult(res);
                }
            }
        }
    }
}
