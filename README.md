# CPPUnitTest2

## Brief
An experiment to enhance the Microsoft C++ Unit Test Framework. 

### History
I was getting extremely frustrated with the MS C++ test framework logging, mostly for the following reasons. Also, I just think that
it's foundation is really cool and they played some neat tricks to expose the test classes and methods, which I wanted to explore.

### Gripes
1) When a C++ exception (std::exception) is thrown, the output in the test window is "an unknown C++ exception occurred". 
  - This seems silly because if it's a standard exception they should at least be able to print the message.
2) When the execution context tries to load a library with dependencies, the message is "failed to setup execution context"
  - This one is a little harder, I haven't fixed it quite yet. But I will.

