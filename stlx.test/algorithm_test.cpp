#include "stdafx.h"
#include "CppUnitTest.h"

#include <stlx/inc/algorithm.h>
#include <CPPUnitTest2/inc/assert.h>

#include <vector>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace stlxtest
{		
	TEST_MODULE_INITIALIZE(module_initialize)
	{
		Logger::WriteMessage("test_module_initialize");
	}

	TEST_MODULE_CLEANUP(module_cleanup)
	{
		Logger::WriteMessage("test_module_initialize");
	}

	TEST_CLASS(algorithm_test)
	{
		std::string member_;
	public:

		algorithm_test()
		{
			Logger::WriteMessage("test_class_ctor");
		}

		~algorithm_test()
		{
			Logger::WriteMessage("test_class_dtor");
		}
				
		TEST_CLASS_INITIALIZE(test_class_init)
		{
			Logger::WriteMessage("test_class_initialize");
		}

		TEST_CLASS_CLEANUP(test_class_cleanup)
		{
			Logger::WriteMessage("test_class_cleanup");
		}
		
		TEST_METHOD_INITIALIZE(algorithm_init)
		{
			member_ = "nope";
			Logger::WriteMessage("test_method_initialize");
		}

		TEST_METHOD_CLEANUP(algorithm_cleanup)
		{
			Logger::WriteMessage("test_method_cleanup");
		}
		
		TEST_METHOD(test_method_1)
		{
			Logger::WriteMessage("test_method_1");
		}

		TEST_METHOD(test_method_2)
		{
			Logger::WriteMessage("test_method_2");
		}

		TEST_METHOD(test_find_vector)
		{
			std::vector<int> vec = { 1,2,3,4,5 };
			int value = stdx::find(vec, 4);
			assert::are_equal(4, value);
		}

		TEST_METHOD(test_not_exists_vector)
		{
			std::vector<char> vec = { 'a','B','c','d','e' };
			assert::is_false(stdx::exists(vec, 'b'));
		}

		TEST_METHOD(test_exists_vector)
		{
			std::vector<std::string> vec = { "The", "Quick", "Brown", "Fox", "Jumps", "Over", "The", "Lazy", "Dog" };
			assert::is_true(stdx::exists(vec, "Quick"));
		}

		TEST_METHOD(test_erase_vector)
		{
			std::vector<char> vec = { 'a','B','c','d','e' };
			stdx::erase(vec, 'B');
			assert::is_false(stdx::exists(vec, 'B'));
		}
		
		TEST_METHOD(test_exists_array)
		{
			int array[4] = { 1, 2, 3, 4 };
			assert::is_true(stdx::exists(array, 3));
		}

		TEST_METHOD(test_exists_ptr)
		{
			// leaky
			int * ptr = new int[3]{ 1, 2, 3 };
			stdx::ptr_container_wrapper<int> pwrp(ptr, ptr + 3);
			assert::is_true(stdx::exists(pwrp, 2));
		}

		TEST_METHOD(test_contains_string_vector)
		{
			std::string qbf("The quick brown fox jumps over the lazy dog.");
			std::vector<char> inc{ 'f','o','x',' ', 'j' };
			assert::is_true(stdx::contains(qbf, inc));
		}

		TEST_METHOD(test_contains_string_string)
		{
			std::string a("A long december");
			std::string b("december");
			assert::is_true(stdx::contains(a, b));
		}

		TEST_METHOD(test_not_contains_string_string)
		{
			std::string a("Counting crows");
			std::string b("ravens");
			assert::is_false(stdx::contains(a, b));
		}

		TEST_METHOD(test_contains_int_vector)
		{
			std::vector<int> v = { 1,2,3,4,5 };
			std::vector<int> v1 = { 2,3,4 };
			assert::is_true(stdx::contains(v, v1));
		}

		TEST_METHOD(test_vector_equals)
		{
			std::vector<int>  v = { 1, 2, 3, 4, 5, 6 };
			std::vector<int> v1 = { 1, 2, 3, 4, 5, 6 };
			assert::is_true(stdx::equals(v1, v));
		}

		TEST_METHOD(test_vector_insert_after)
		{
			std::vector<int> v = {1, 2, 4, 5};
			std::vector<int> v1 = { 1, 2,3,4,5 };

			//assert::is_true(stdx::insert_after(v, 2, 3));
			assert::is_true(stdx::equals(v, v1));
		}

		TEST_METHOD(test_vector_insert_before)
		{
			std::vector<int> v = { 1, 2, 4, 5 };
			std::vector<int> v1 = { 1,2,3,4,5 };

			//assert::is_true(stdx::insert_before(v, 4, 3));
			assert::is_true(stdx::equals(v, v1));
		}
	};
}