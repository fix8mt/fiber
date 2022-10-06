#include <iostream>
#include <thread>
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
	f8_fiber([]()
	{
		int ii{};
		do print_a(); while (ii++ < 1000);
	}).detach();

	f8_fiber([]()
	{
		int ii{};
		do print_b(); while (ii++ < 1000);
	}).detach();

	fibers::print();
	return 0;
}

int main()
{
	test();
	std::cout << "xxxx";
	return 0;
}
