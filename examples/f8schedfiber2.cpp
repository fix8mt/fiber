#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <fix8/f8schedfiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct foo
{
	void sub(int arg, std::promise<int>& pr)
	{
		std::cout << "\tstarting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		pr.set_value(arg * 100);
		std::cout << "\tleaving " << arg << '\n';
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	try
	{
		foo bar;
		std::promise<int> mypromise;
		auto myfuture { mypromise.get_future() };
		f8_sched_fiber sub_co(&foo::sub, &bar, 10, std::ref(mypromise));
		for (int ii{}; sub_co; )
		{
			std::cout << "main: " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "Exiting from main\n";
		std::cout << "Future result = " << myfuture.get() << '\n';
		std::cout << "Repeated result (should throw exception) = " << myfuture.get() << '\n';
	}
	catch (const std::future_error& e)
	{
		std::cerr << "Exception: " << e.what() << '(' << e.code() << ")\n";
	}
	return 0;
}
