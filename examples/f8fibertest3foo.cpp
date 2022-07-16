//-----------------------------------------------------------------------------------------
// f8_fiber (header only) based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <string>

#include <fix8/f8fiber.hpp>
#include "f8fibertest3.hpp"

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
f8_fiber foo::func (f8_fiber&& f, bool& flags)
{
	std::cout << "\tfunc:entry\n";
	std::cout << "\tcaller id:" << f.get_id() << '\n';
	for (int kk{}; kk < _cnt; ++kk)
	{
		std::cout << "\tfunc:" << kk << '\n';
		f8_yield(f);
		std::cout << "\tfunc:" << kk << " (resumed)\n";
	}
	flags = true;
	std::cout << "\tfunc:exit\n";
	return std::move(f);
}

