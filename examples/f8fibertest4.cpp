#include <iostream>
#include <functional>
#include <future>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct foo
{
	int sub(int arg)
	{
		fibers::print(std::cout);
		std::cout << "\tstarting " << arg << '\n';
		for (int ii{}; ii < arg; )
			std::cout << '\t' << arg << ": " << ++ii << '\n';
		std::cout << "\tleaving " << arg << '\n';
		return arg * 100;
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	try
	{
		foo bar;
		std::future<int> myfuture { async(&foo::sub, &bar, 10) };
		std::cout << "Future result = " << myfuture.get() << '\n';
		fibers::print(std::cout);
	}
	catch (const std::future_error& e)
	{
		std::cerr << "Exception: " << e.what() << '(' << e.code() << ")\n";
	}
	std::cout << "Exiting from main\n";
	return 0;
}
