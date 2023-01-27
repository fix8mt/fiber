//-----------------------------------------------------------------------------------------
// fiber (header only)
// Copyright (C) 2022-23 Fix8 Market Technologies Pty Ltd
//   by David L. Dight
// see https://github.com/fix8mt/fiber
//
// Lightweight header-only stackful per-thread fiber
//		with built-in roundrobin scheduler x86_64 / linux only
//
// Distributed under the Boost Software License, Version 1.0 August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <string_view>
#include <array>
#include <list>
#include <utility>
#include <fix8/fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
/*
void Fibonacci(int& val)
{
	val = 0;
	this_fiber::yield(); // F(0) -> 0
	int prev{}, curr{1};
	while (true)
	{
		val = curr;
		this_fiber::yield();
		int tmp { prev + curr };
		prev = curr;
		curr = tmp;
	}
}

int main()
{
	int val{};
	fiber fib(Fibonacci, std::ref(val));
	while (val < 1597)
	{
		fib.resume();
		std::cout << val << '\n';
	}
	fib.kill();

	return 0;
}
*/

int main(int argc, char *argv[])
{
	int num{18};
	if (argc > 1) try
	{
		num = std::stoi(argv[1]);
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception (" << e.what() << "): " << argv[1] << std::endl;
		exit(1);
	}
	uint64_t val{};
	fiber fib {[&val]()
	{
		for(uint64_t next{1};; val = std::exchange(next, val + next))
			this_fiber::yield();
	}};

	while (num--)
	{
		fib.resume();
		std::cout << val << ' ';
	}
	fib.kill();
	std::cout << "\nmain: done\n";
	return 0;
}

