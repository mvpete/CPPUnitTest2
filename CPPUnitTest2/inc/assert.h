#ifndef __assert_h__
#define __assert_h__

namespace assert
{
	struct __line_info
	{
		const char *filename;
		const char *function;
		int line;

		__line_info()
		{
		}

		__line_info(const char *filename, const char *function, int line)
			:filename(filename), function(function), line(line)
		{
		}
	};

	#define LINE_INFO() &__line_info(__FILE__,__FUNCTION__,__LINE__)

	class assert_exception : public std::exception
	{
		__line_info info_;
	public:
		assert_exception(const char *msg, const __line_info *li)
			:std::exception(msg==nullptr?"Assert Failed":msg), info_(li!=nullptr?*li:__line_info())
		{
		}

		static void make_error(const char *msg=nullptr, const __line_info *li=nullptr)
		{
			throw assert_exception(msg, li);
		}
	};

	void fail_on_condition(bool condition, const char *msg, const __line_info *li)
	{
		if (condition)
			assert_exception::make_error(msg, li);
	}


	void is_true(bool condition, const char *msg = nullptr, const __line_info *li = nullptr)
	{
		fail_on_condition(!condition, msg, li);
	}

	void is_false(bool condition, const char *msg = nullptr, const __line_info *li = nullptr)
	{
		fail_on_condition(condition, msg, li);
	}

	template <typename T> 
	void are_equal(const T& expected, const T& actual, const char *msg = nullptr, const __line_info *li = nullptr)
	{
		fail_on_condition(!(expected == actual), msg, li);
	}



}



#endif // __assert_h__
