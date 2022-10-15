#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8fiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct foo
{
	void sub(int arg)
	{
		std::cout << "\tstarting " << arg << '\n';
		for (int ii{}; ii < arg; )
			std::cout << '\t' << arg << ": " << ++ii << '\n';
		std::cout << "\tleaving " << arg << '\n';
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	try
	{
		foo bar;
		fiber sub_co(&foo::sub, &bar, 10);
		std::cout << "main\n";
		this_fiber::yield();
		fibers::print();
		std::cout << "Exiting from main\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "\nException: " << e.what() << '\n';
	}
	return 0;
}
