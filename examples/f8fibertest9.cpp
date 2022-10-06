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
struct foo : f8_fiber
{
	foo(const char *str) : f8_fiber(&foo::sub, this, str) {}

	void sub(const char *str)
	{
		std::cout << "\tstarting " << this_fiber::name(str) << '\n';
		for (int ii{}; ii < 5; )
		{
			std::cout << '\t' << str << ": " << std::dec << ++ii << '\n';
			this_fiber::yield();
		}
		fibers::print();
		std::cout << "\tleaving " << str << '\n';
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
