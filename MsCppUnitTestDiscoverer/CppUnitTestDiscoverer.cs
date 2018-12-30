using Microsoft.VisualStudio.TestPlatform.ObjectModel;
using Microsoft.VisualStudio.TestPlatform.ObjectModel.Adapter;
using Microsoft.VisualStudio.TestPlatform.ObjectModel.Logging;
using MsCppUnitTestAdapter;
using System;
using System.Collections.Generic;
using System.Linq;

namespace MsCppUnitTestDiscoverer
{
    [FileExtension(".dll")]
    [FileExtension(".exe")]
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
                    var tp = TestProperty.Find("ClassName") ?? TestProperty.Register("ClassName", "ClassName", typeof(string), typeof(TestCase));
                    tc.SetPropertyValue(tp, test.ClassName);

                    tc.Traits.Add(new Trait("ClassName", "test"));
                    cases.Add(tc);
                }
            }
            return cases;
        }

        private void ExecuteTests(IEnumerable<TestCase> tests, IFrameworkHandle frameworkHandle)
        {
            // sort the tests by source. easier in .NET
            var ordered = tests.GroupBy(tc => tc.Source);

            TestResult result = null;
            foreach (var srcList in ordered)
            {
                try
                {
                    // Setup Execution Context
                    using (var ectx = new MsCppUnitTestAdapter.VsTestAdapterExecutionContext(srcList.Key))
                    {
                        var classes = srcList.GroupBy(tc => tc.GetPropertyValue<String>(TestProperty.Find("ClassName"), ""));
                        
                        foreach (var testClassList in classes)
                        {
                            // initialize class
                            ectx.LoadClass(testClassList.Key);
                            foreach (var test in testClassList)
                            {                             
                                frameworkHandle.RecordStart(test);

                                result = new TestResult(test);
                                ectx.ExecuteMethod(test.FullyQualifiedName, result);
                                frameworkHandle.RecordResult(result);
                            }
                            ectx.UnloadClass(testClassList.Key);
                        }
                    }
                }
                catch (Exception e)
                {
                    if (result == null)
                        continue;

                    result.EndTime = DateTimeOffset.Now;
                    result.Duration = result.EndTime - result.StartTime;
                    result.Outcome = TestOutcome.Failed;
                    result.ErrorMessage = e.Message;
                    result.ErrorStackTrace = e.StackTrace;
                    frameworkHandle.RecordResult(result);
                }
            }
        }
    }
}
