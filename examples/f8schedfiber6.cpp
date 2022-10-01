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
	printf("a");
	this_fiber::yield();
}

void print_b()
{
	printf("b");
	std::thread j([]() { printf("N"); });
	j.detach();
	this_fiber::yield();
}

int test()
{
	f8_sched_fiber([]()
	{
		do
		{
			print_a();
		}
		while (true);
	}).detach();

	f8_sched_fiber([]()
	{
		do
		{
			print_b();
		}
		while (true);
	}).detach();

	printf("xxxx");

	return 0;
}

int main()
{
	test();
	return 0;
}
