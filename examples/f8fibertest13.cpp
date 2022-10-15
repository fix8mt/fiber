#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8fiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
struct blah
{
	~blah() { std::cout << "~blah() - " << this_fiber::name() << '\n'; }
};
void doit(int arg)
{
	blah b;
	std::cout << "\tstarting " << this_fiber::name() << ' ' << arg << '\n';
	for (int ii{}; ii < arg; )
	{
		std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
		this_fiber::yield();
	}
	std::cout << "\tleaving " << this_fiber::name() << ' ' << arg << '\n';
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	fiber sub_co({.name="sub0"}, &doit, 9), sub_co1({.name="sub1"}, &doit, 10);
	fibers::print();
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		if (ii == 2)
		{
			fibers::print();
			sub_co1.suspend();
			fibers::print();
		}
		if (ii == 6)
		{
			fibers::print();
			sub_co1.unsuspend();
			fibers::print();
		}
		this_fiber::yield();
		std::cout << "main: " << std::dec << ii << '\n';
	}
	std::cout << "Exiting from main\n";
	fibers::print();
	return 0;
}
