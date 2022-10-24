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
	fiber sub_co({.stacksz=2048}, &doit, 3),
			sub_co1({.stacksz=16384}, &foo::sub2, &bar),
			sub_co2({.stacksz=32768}, &foo::sub, &bar, 5),
			sub_co3({.stacksz=8192}, &foo::sub1, &bar, 8., "hello"),
			sub_co4(std::bind(&foo::sub3, &bar, 12, "there"));
	fiber sub_lam({.name="sub lambda",.stack=std::make_unique<f8_fixedsize_mapped_stack>()}, [](int arg)
	//char stack[4096];
	//fiber sub_lam({.name="sub lambda",.stacksz=sizeof(stack),.stack=std::make_unique<f8_fixedsize_placement_stack>(stack)}, [](int arg)
	{
		std::cout << "\tlambda starting " << arg << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << '\t' << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "\tlambda leaving " << arg << '\n';
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
	std::cout << "Exiting from main\nSizes\n";
	std::cout << "fiber: " << sizeof(fiber) << '\n';
	std::cout << "fiber::cvars: " << sizeof(fiber::cvars) << '\n';
	std::cout << "fiber_id: " << sizeof(fiber_id) << '\n';
	std::cout << "fiber_base: " << sizeof(fiber_base) << '\n';
	std::cout <<"fiber_params: " << sizeof(fiber_params) << '\n';
	return 0;
}
