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
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main()
{
	static constexpr const std::array<std::array<std::string_view, 6>, 4> wordset
	{{
		{	R"("I )",		"all ",		"said ",		"It’s ",		"I’m ",								},
		{	"for ",			"who ",		"me. ",		"them ",		"myself.\"\n"						},
		{	"am ",			"of ",		"no ",		"because ",	"doing ",		" - Albert ",	},
		{	"thankful ",	"those ",	"to ",		"of ",		"it ",			"Einstein\n"	},
	}};

	const auto func([](const auto& words)
	{
		for (auto pp : words)
		{
			std::cout << pp;
			this_fiber::yield();
		}
	});

	std::list<fiber> sts;

	launch_all_n
	(
	 	sts,
		std::bind(func, wordset[0]),
		std::bind(func, wordset[1]),
		std::bind(func, wordset[2]),
		std::bind(func, wordset[3])
	);
	fibers::wait_all();

	sts.clear();
	fibers::set_flag(global_fiber_flags::skipmain);

	launch_all_with_params_n
	(
	 	sts,
		fiber_params{.launch_order=0}, std::bind(func, wordset[0]),
		fiber_params{.launch_order=3}, std::bind(func, wordset[1]),
		fiber_params{.launch_order=1}, std::bind(func, wordset[2]),
		fiber_params{.launch_order=2}, std::bind(func, wordset[3])
	);

	return 0;
}
