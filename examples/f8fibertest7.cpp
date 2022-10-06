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
	std::thread([]() { std::cout << 'B'; }).detach();
	this_fiber::yield();
}

int main()
{
	int ii{};

	f8_fiber([&ii]()
	{
		do print_a();
		while (++ii < 20);
	}).detach();
	f8_fiber([&ii]()
	{
		do print_b();
		while (++ii < 20);
	}).detach();

	std::cout << 'X';
	return 0;
}

// XababBabBabBababBBabBabBabBabBB
// XababBabBabBabBabBabBabaBbBabBB
// XababBabBabBabBabBabBabBabBabBB
