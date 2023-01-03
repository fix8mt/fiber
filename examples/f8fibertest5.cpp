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
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct blah
{
	~blah() { std::cout << "~blah()\n"; }
};

struct foo
{
	void sub(int arg, std::promise<int>& pr)
	{
		blah b;
		try
		{
			std::cout << "\tstarting " << arg << '\n';
			for (int ii{}; ii < arg; )
			{
				std::cout << '\t' << arg << ": " << ++ii << '\n';
				this_fiber::yield();
			}
			//pr.set_value(arg * 100);
			std::cout << "\tleaving " << arg << '\n';
			throw std::runtime_error("test exception");
		}
		catch (...)
		{
			try
			{
				pr.set_exception(std::current_exception());
			}
			catch(...)
			{
				std::cerr << "pr.set_exception(std::current_exception()) threw\n";
			}
		}
	}
	~foo() { std::cout << "~foo()\n"; }
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	foo bar;
	std::promise<int> mypromise;
	auto myfuture { mypromise.get_future() };
	fiber sub_co(&foo::sub, &bar, 10, std::ref(mypromise));
	for (int ii{}; sub_co; )
	{
		std::cout << "main: " << ++ii << '\n';
		this_fiber::yield();
	}
	try
	{
		std::cout << "Future result = " << myfuture.get() << '\n';
		sub_co.join_if();
	}
	catch (const std::exception& e)
	{
		std::cerr << "\nException: " << e.what() << '\n';
	}
	std::cout << "Exiting from main\n";
	return 0;
}
