#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <thread>
#include <unistd.h>
#include <fix8/f8schedfiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

void print_a()
{
	std::cout << "a";
	this_fiber::yield();
}

void print_b()
{
	std::cout << "b";
	std::thread j([]() { std::cout << "B"; });
	j.detach();
	this_fiber::yield();
}

int main()
{
	int i = 0;
	f8_sched_fiber([&]()
	{
		this_fiber::name("first");
		do
		{
			print_a();
			i++;
		}
		while (i < 20);
	}).detach();

	f8_sched_fiber([&]()
	{
		this_fiber::name("second");
		do
		{
			i++;
			print_b();
		}
		while (i < 20);
	}).detach();

	//fibers::print(std::cout);
	std::cout << "X";
	return 0;
}

// XababBabBabBababBBabBabBabBabBB
// XababBabBabBabBabBabBabaBbBabBB
// XababBabBabBabBabBabBabBabBabBB
