#include <iostream>
#include <thread>
#define FIBER_NO_MULTITHREADING
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

void print_a()
{
	std::cout << 'a';
	this_fiber::yield();
}

void print_b()
{
	std::cout << 'b';
	std::thread([]() { std::cout << 'N'; }).detach();
	this_fiber::yield();
}

int test()
{
	fiber([]()
	{
		int ii{};
		do print_a(); while (ii++ < 1000);
	}).detach();

	fiber([]()
	{
		int ii{};
		do print_b(); while (ii++ < 1000);
	}).detach();
	return 0;
}

int main()
{
	test();
	std::cout << "xxxx";
	return 0;
}
