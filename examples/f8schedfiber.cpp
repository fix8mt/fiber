#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8schedfiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void doit(int arg)
{
	std::cout << "\tstarting " << arg << '\n';
	for (int ii{}; ii < arg; )
	{
		std::cout << '\t' << arg << ": " << ++ii << '\n';
		this_fiber::yield();
	}
	std::cout << "\tleaving " << arg << '\n';
}

struct foo
{
	void sub(int arg) { doit(arg); }
	void sub1(int arg, const char *str)
	{
		std::cout << str << '\n';
		doit(arg);
	}
	void sub3(int arg, const char *str)
	{
		using namespace std::literals;
		std::cout << "\tsub2 starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			//this_fiber::sleep_until(std::chrono::steady_clock::now() + 500ms);
			this_fiber::sleep_for(100ms);
		}
		std::cout << "\tsub2 leaving " << arg << '\n';
	}
	void sub2() { doit(3); }
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	foo bar;
	f8_sched_fiber sub_co(&doit, 5), sub_co1(&foo::sub2, &bar), sub_co2(&foo::sub, &bar, 8), sub_co3(&foo::sub1, &bar, 12., "hello"),
		sub_co4(std::bind(&foo::sub3, &bar, 15, "there"), 32767);
	f8_sched_fiber sub_lam([](int arg)
	{
		std::cout << "\tlam starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "\tlam leaving " << arg << '\n';
	}, 18);
	fibers::print(std::cout);
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		this_fiber::yield();
		std::cout << "main: " << ii << '\n';
		//fibers::print(std::cout);
	}
	std::cout << "Exiting from main\n";
	std::cout << sizeof(f8_sched_fiber) << '\n';
	return 0;
}
