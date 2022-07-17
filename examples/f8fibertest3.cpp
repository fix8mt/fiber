//-----------------------------------------------------------------------------------------
// f8_fiber (header only) based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <string>

//#define F8FIBER_USE_ASM_SOURCE
#include <fix8/f8fiber.hpp>
#include "f8fibertest3.hpp"

//F8FIBER_ASM_SOURCE;

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	bool flags{};
	foo bar(argc > 1 ? std::stol(argv[1]) : 5);
	f8_fiber f0(std::bind(&foo::func, &bar, std::placeholders::_1, std::ref(flags)));
	std::cout << "fiber id:" << f0.get_id() << '\n';
	std::cout << "flags=" << std::boolalpha << flags << '\n';

	for (int ii{}; f0; ++ii)
	{
		std::cout << "main:" << ii << '\n';
		f8_yield(f0);
		std::cout << "main:" << ii << " (resumed)\n";
	}
	std::cout << "flags=" << std::boolalpha << flags << '\n';
	std::cout << "main:exit\n";
	return 0;
}

