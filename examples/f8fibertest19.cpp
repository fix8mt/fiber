#include <iostream>
#include <functional>
#include <deque>
#include <array>
#include <fix8/f8fiber.hpp>

// CC=gcc CXX="g++ -ggdb" CXXFLAGS=-O0 -DCMAKE_BUILD_TYPE=Debug cmake ..
//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
void func(int arg)
{
	const std::string tag { std::string(this_fiber::name()) + ' ' + std::to_string(arg) };
	std::cout << "\tstarting " << tag << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
		std::cout << "\t\t" << tag << ": " << ++ii << '\n';
	std::cout << "\tleaving " << tag << '\n';
	fibers::print();
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	auto stack_memory { std::make_unique<char[]>(32768) };
	std::array<fiber, 12> fbs
	{{
		{ {.name="sub01",.stacksz=8192}, &func, 3 },
		{ {.name="sub02",.stacksz=8192}, &func, 6 },
		{ {.name="sub03",.stacksz=8192}, &func, 9 },
		{ {.name="sub04",.stacksz=8192}, &func, 12 },
		{ {.name="sub05",.stacksz=8192,.stack=make_stack<stack_type::placement>(stack_memory.get())}, &func, 3 },
		{ {.name="sub06",.stacksz=8192,.stack=make_stack<stack_type::placement>(stack_memory.get(), 8192)}, &func, 6 },
		{ {.name="sub07",.stacksz=8192,.stack=make_stack<stack_type::placement>(stack_memory.get(), 2 * 8192)}, &func, 9 },
		{ {.name="sub08",.stacksz=8192,.stack=make_stack<stack_type::placement>(stack_memory.get(), 3 * 8192)}, &func, 12 },
		{ {.name="sub09",.stacksz=8192,.stack=make_stack<stack_type::mapped>()}, &func, 3 },
		{ {.name="sub10",.stacksz=8192,.stack=make_stack<stack_type::mapped>()}, &func, 6 },
		{ {.name="sub11",.stacksz=8192,.stack=make_stack<stack_type::mapped>()}, &func, 9 },
		{ {.name="sub12",.stacksz=8192,.stack=make_stack<stack_type::mapped>()}, &func, 12 }
	}};
	fibers::print();
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		std::cout << "main: " << std::dec << ii << '\n';
		this_fiber::yield();
	}
	std::cout << "Exiting from main\n";
	return 0;
}
