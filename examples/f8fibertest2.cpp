#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main(void)
{
	try
	{
		std::promise<int> mypromise;
		auto myfuture { mypromise.get_future() };
		f8_fiber sub_co([](int arg, std::promise<int>& pr)
		{
			std::cout << "\tstarting " << arg << '\n';
			for (int ii{}; ii < arg; )
			{
				std::cout << '\t' << arg << ": " << ++ii << '\n';
				this_fiber::yield();
			}
			pr.set_value(arg * 100);
			std::cout << "\tleaving " << arg << '\n';
		}, 10, std::ref(mypromise));

		for (int ii{}; sub_co; )
		{
			std::cout << "main: " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "Exiting from main\n";
		std::cout << "Future result = " << myfuture.get() << '\n';
		//if (myfuture.valid())
		std::cout << "Repeated result (should throw exception) = " << myfuture.get() << '\n';
	}
	catch (const std::future_error& e)
	{
		std::cerr << "Exception: " << e.what() << '(' << e.code() << ")\n";
	}
	return 0;
}
