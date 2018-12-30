CPP Unit Test Investigator
===

I forgot how this thing goes together. So I need to write it down for next time.

Build
---

Build solution - projects will output to sln root Debug.

Deploy
---

From output folder, copy 

   - CPPUnitTestInvestigator.dll
   - MsCppUnitTestAdapter.dll
   - MsCppUnitTestDiscoverer.dll

To 
- 2015 - C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\CommonExtensions\Microsoft\TestWindow\Extensions
- 2017 - C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow\Extensions
Kill vstest.discovery.engine.x86.exe


ToDos
---

- Random ass VS crash
- Class names don't work
- Assert SEH are not handled. Would be nice to play nice with Ms Assert. Mine uses C++ exception