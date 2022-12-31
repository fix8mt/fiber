//-----------------------------------------------------------------------------------------
// fiber (header only)
// Copyright (C) 2022-23 Fix8 Market Technologies Pty Ltd
//   by David L. Dight
// see https://github.com/fix8mt/f8fiber
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
#include <thread>
#include <chrono>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
class blah
{
	std::string_view tabs;
public:
	blah() = default;
	blah(std::string_view tb) : tabs(std::move(tb)) {}
	~blah() { std::cout << tabs << "~blah(): " << this_fiber::name() << '\n'; }
};

void doit_with_stoprequest(bool& stop_requested)
{
	blah b("\t");
	bool waitagain{};
	std::cout << '\t' << "Starting " << this_fiber::name() << '\n';
	for(int ii{};; this_fiber::yield())
	{
		std::cout << '\t' << this_fiber::name() << ": " << ++ii << '\n';
		if (stop_requested)
		{
			if (waitagain)
			{
				std::cout << '\t' << this_fiber::name() << ": stop actioned\n";
				break;
			}
			else
			{
				std::cout << '\t' << this_fiber::name() << ": stop requested\n";
				waitagain = true;
			}
		}
	}
	std::cout << '\t' << "Leaving " << this_fiber::name() << '\n';
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	std::cout << "Starting " << this_fiber::name() << '\n';
	blah b;
	bool stop_requested{};
	fiber sub_co({.name="sub",.join=true}, &doit_with_stoprequest, std::ref(stop_requested));
	for (int ii{}; ii < 5; this_fiber::yield())
	{
		std::cout << this_fiber::name() << ": " << ++ii << '\n';
		std::this_thread::sleep_for(100ms);
	}
	stop_requested = true;
	this_fiber::yield();
	fibers::print();
	std::cout << "Exiting " << this_fiber::name() << '\n';
	return 0;
}
