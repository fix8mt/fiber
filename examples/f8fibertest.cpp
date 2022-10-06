#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8fiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
void doit(int arg)
{
	std::cout << this_fiber::name(("sub"s + std::to_string(arg)).c_str());
	std::cout << "\tstarting " << arg << '\n';
	for (int ii{}; ii < arg; )
	{
		std::cout << '\t' << arg << ": " << ++ii << '\n';
		this_fiber::yield();
	}
	std::cout << "\tleaving " << arg << '\n';
	fibers::print();
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
	f8_fiber sub_co(&doit, 3), sub_co1(&foo::sub2, &bar), sub_co2(&foo::sub, &bar, 5), sub_co3(&foo::sub1, &bar, 8., "hello"),
		sub_co4(std::bind(&foo::sub3, &bar, 12, "there"), 32767);
	f8_fiber sub_lam({.name="sub lambda"}, [](int arg)
	{
		std::cout << "\tlam starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "\tlam leaving " << arg << '\n';
	}, 15);
	fibers::print();
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		if (ii == 0)
		{
			fibers::print(std::cout);
			sub_co3.resume();
			fibers::print(std::cout);
		}
		this_fiber::yield();
		std::cout << "main: " << std::dec << ii << '\n';
		//fibers::print(std::cout);
	}
	std::cout << "Exiting from main\n";
	std::cout << sizeof(f8_fiber) << '\n';
	std::cout << sizeof(f8_fiber_base) << '\n';
	return 0;
}
