#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#define FIBER_NO_INSTRUMENTATION
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct foo
{
	int sub(int arg)
	{
		std::cout << "\tstarting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
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
		std::packaged_task<int(int)> task(std::bind(&foo::sub, &bar, std::placeholders::_1));
		std::future<int> myfuture { task.get_future() };
		fiber sub_co(std::move(task), 10);
		for (int ii{}; sub_co; )
		{
			std::cout << "main: " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "Exiting from main\n";
		std::cout << "Future result = " << myfuture.get() << '\n';
	}
	catch (const std::future_error& e)
	{
		std::cerr << "Exception: " << e.what() << '(' << e.code() << ")\n";
	}
	return 0;
}
