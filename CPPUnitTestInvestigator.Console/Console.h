#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <string>
#include <functional>
#include <vector>

class Console
{
public:
	static void WriteLine(const std::string &msg);
	static void Write(const std::string &msg);
	static std::string ReadLine();
	static std::string ReadKey();
};

class Args
{
	std::vector<std::string> args_;
public:
	Args(const std::string &line);
	const std::string & operator[](int i) const;
	size_t Size() const;
};

using CommandFn = std::function<void(const Args &)>;

class CommandLineInterface
{
	std::function<std::string, CommandFn> functions_;

	void OnInput(const std::string &line);

protected:
	void RegisterFunction(const std::string &fn, CommandFn fn)
public:
	CommandLineInterface(const std::string &name);
	void Run();
};


#endif // __CONSOLE_H__
