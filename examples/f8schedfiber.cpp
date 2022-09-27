#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8schedfiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
void doit(int arg)
{
	this_fiber::name(("sub"s + std::to_string(arg)).c_str());
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
	void sub(int arg)
	{
		doit(arg);
	}
	void sub1(int arg, const char *str)
	{
		std::cout << str << '\n';
		doit(arg);
	}
	void sub3(int arg, const char *str)
	{
		auto st { "sub"s + std::to_string(arg) };
		//std::cout << st << '\n';
		this_fiber::name(st.c_str());
		std::cout << "\tsub2 starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			//this_fiber::sleep_until(std::chrono::steady_clock::now() + 500ms);
			this_fiber::sleep_for(100ms);
		}
		std::cout << "\tsub2 leaving " << arg << '\n';
	}
	void sub2()
	{
		doit(4);
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	foo bar;
	f8_sched_fiber sub_co(&doit, 3), sub_co1(&foo::sub2, &bar), sub_co2(&foo::sub, &bar, 5), sub_co3(&foo::sub1, &bar, 8., "hello"),
		sub_co4(std::bind(&foo::sub3, &bar, 12, "there"), 32767);
	f8_sched_fiber sub_lam([](int arg)
	{
		this_fiber::name("sub lambda");
		std::cout << "\tlam starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "\tlam leaving " << arg << '\n';
	}, 15);
	fibers::print(std::cout);
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		if (ii == 0)
		{
			fibers::print(std::cout);
			this_fiber::yield(sub_co3.get_id());
			fibers::print(std::cout);
		}
		this_fiber::yield();
		std::cout << "main: " << ii << '\n';
		//fibers::print(std::cout);
	}
	std::cout << "Exiting from main\n";
	std::cout << sizeof(f8_sched_fiber) << '\n';
	return 0;
}
