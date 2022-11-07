#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8fiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
class foo : public fiber
{
	int _arg;

public:
	foo(int arg) : fiber (&foo::sub, this), _arg(arg) {}

	void sub()
	{
		std::cout << "\tstarting sub " << _arg << '\n';
		for (int ii{}; ii < _arg; this_fiber::yield())
			std::cout << '\t' << _arg << ": " << ++ii << '\n';
		std::cout << "\tleaving sub " << _arg << '\n';
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	try
	{
		foo bar(10);
		fibers::print();
		while (fibers::has_fibers())
		{
			std::cout << "main\n";
			this_fiber::yield();
		}
		fibers::print();
		std::cout << "Exiting from main\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "\nException: " << e.what() << '\n';
	}
	return 0;
}
