#include <iostream>
#include <functional>
#include <future>
#define FIBER_NO_INSTRUMENTATION
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main(void)
{
	struct foo
	{
		int sub(int arg)
		{
			std::cout << "\tstarting " << arg << '\n';
			for (int ii{}; ii < arg; this_fiber::yield())
				std::cout << '\t' << arg << ": " << ++ii << '\n';
			std::cout << "\tleaving " << arg << '\n';
			return arg * 100;
		}
	} bar;

	std::packaged_task<int(int)> task(std::bind(&foo::sub, &bar, std::placeholders::_1));
	std::future<int> myfuture { task.get_future() };
	fiber sub_co(std::move(task), 10);
	for (int ii{}; sub_co; this_fiber::yield())
		std::cout << "main: " << ++ii << '\n';
	std::cout << "Exiting from main\n";
	std::cout << "Future result = " << myfuture.get() << '\n';
	return 0;
}
