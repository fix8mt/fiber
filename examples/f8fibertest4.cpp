#include <iostream>
#include <functional>
#include <future>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int sub(int arg)
{
	fibers::print();
	std::cout << "\tstarting " << arg << '\n';
	for (int ii{}; ii < arg; )
		std::cout << '\t' << arg << ": " << ++ii << '\n';
	std::cout << "\tleaving " << arg << '\n';
	return arg * 100;
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	std::future<int> myfuture { async(&sub, 10) };
	std::cout << "Future result = " << myfuture.get() << '\n';
	fibers::print();
	std::cout << "Exiting from main\n";
	return 0;
}
