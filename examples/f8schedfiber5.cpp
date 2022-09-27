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
		try
		{
			std::cout << "\tstarting " << arg << '\n';
			for (int ii{}; ii < arg; )
			{
				std::cout << '\t' << arg << ": " << ++ii << '\n';
				this_fiber::yield();
			}
			pr.set_value(arg * 100);
			std::cout << "\tleaving " << arg << '\n';
			//throw std::runtime_error("test exception");
		}
		catch (...)
		{
			try
			{
				pr.set_exception(std::current_exception());
			}
			catch(...)
			{
				std::cerr << "pr.set_exception(std::current_exception()) threw\n";
			}
		}
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
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
	try
	{
		std::cout << "Future result = " << myfuture.get() << '\n';
		sub_co.join();
	}
	catch (const std::exception& e)
	{
		std::cerr << "\nException: " << e.what() << '\n';
	}
	std::cout << "Exiting from main\n";
	return 0;
}
