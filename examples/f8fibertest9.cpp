#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <string>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
struct foo : fiber
{
	foo(const char *str) : fiber(&foo::sub, this) { set_params(str); }

	void sub()
	{
		std::cout << "\tstarting " << this_fiber::name() << '\n';
		for (int ii{}; ii < 5; )
		{
			std::cout << '\t' << this_fiber::name() << ": " << std::dec << ++ii << '\n';
			this_fiber::yield();
		}
		fibers::print();
		std::cout << "\tleaving " << this_fiber::name() << '\n';
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	foo sub_co("sub_co"), sub_co1("sub_co1"), sub_co2("sub_co2");
	for (int ii{}; fibers::has_fibers(); )
	{
		std::cout << "main: " << std::dec << ++ii << '\n';
		sub_co.resume_if();
		this_fiber::yield();
	}
	std::cout << "Exiting from main\n";
	fibers::print();
	return 0;
}
