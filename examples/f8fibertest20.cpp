#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <thread>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::chrono_literals;

//-----------------------------------------------------------------------------------------
std::thread::id id0;

//-----------------------------------------------------------------------------------------
void sub(int arg)
{
	id0 = std::this_thread::get_id();

	std::cout << "\tstarting sub " << arg << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
	{
		std::cout << "\tsub " << std::this_thread::get_id() << ' ' << arg << ": " << ++ii << '\n';
		std::this_thread::sleep_for(500ms);
	}
	std::cout << "\tleaving sub " << arg << '\n';
}

void sub1(int arg)
{
	std::cout << "\tstarting sub1 " << arg << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
	{
		std::cout << "\tsub1 " << std::this_thread::get_id() << ' ' << arg << ": " << ++ii << '\n';
		if (ii == 5)
		{
			std::cout << "\ntransferring from " << std::this_thread::get_id() << " to " << id0 << '\n';
			this_fiber::move(id0);
		}
		std::this_thread::sleep_for(500ms);
	}
	std::cout << "\tleaving sub1 " << arg << '\n';
}

void sub2(int arg)
{
	std::cout << "\tstarting sub2 " << arg << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
	{
		std::cout << "\tsub2 " << std::this_thread::get_id() << ' ' << arg << ": " << ++ii << '\n';
		std::this_thread::sleep_for(500ms);
	}
	std::cout << "\tleaving sub2 " << arg << '\n';
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	fiber f0({.name="first"},&sub, 15);
	std::thread t1([]()
	{
		fiber f1({.name="second"},&sub1, 12), f2({.name="third"},&sub2, 13);
		while (fibers::has_fibers())
		{
			std::cout << "main1\n";
			//fibers::print();
			this_fiber::yield();
		}
		std::cout << "Exiting from main1\n";
	});
	std::this_thread::sleep_for(100ms);
	while (fibers::has_fibers())
	{
		std::cout << "main\n";
		//fibers::print();
		this_fiber::yield();
	}
	std::cout << "Exiting from main\n";
	t1.join();
	return 0;
}
