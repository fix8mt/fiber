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
	std::packaged_task<int(int)> task(std::bind([](int arg)
	{
		std::cout << "\tstarting sub\n";
		for (int ii{}; ii < arg; this_fiber::yield())
			std::cout << "\tsub: " << ++ii << '\n';
		std::cout << "\tleaving sub\n";
		return arg * 100;
	}, std::placeholders::_1));
	auto myfuture { task.get_future() };
	fiber myfiber(std::move(task), 10);
	for (int ii{}; myfiber; this_fiber::yield())
		std::cout << "main: " << ++ii << '\n';
	std::cout << "Future result = " << myfuture.get() << "\nExiting from main\n";
	return 0;
}
