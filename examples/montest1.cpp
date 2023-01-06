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
//
// Monitor test using:
//
// Termbox2
//		MIT License
// 	Copyright (c) 2010-2020 nsf <no.smile.face@gmail.com>
//		              2015-2022 Adam Saponara <as@php.net>
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <fix8/fiber.hpp>
#include <fix8/fibermonitor.hpp>
#include <unistd.h>
#ifdef _GNU_SOURCE
#include <getopt.h>
#endif

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
class foo
{
	std::mt19937_64 _rnde {std::random_device{}()};
	std::uniform_int_distribution<int> _dist{1, 224};
	fiber_monitor& _fm;
	int _sleepval, _val;
	window_frame _xyxy;

public:
	foo(fiber_monitor& fm, int sleepval) : _fm(fm), _sleepval(sleepval),
		_xyxy({{}, {_fm.get_dimensions().first, _fm.get_dimensions().second}}) {}

	void func(int arg)
	{
		while (arg--)
		{
			double varr[arg * _dist(_rnde)]; // consume variable amt of stack
			_val = varr[_dist(_rnde)]; // force optimizer hands off

			_fm.update(_xyxy);
			if (!_fm)
				this_fiber::resume_main();
			std::this_thread::sleep_for(std::chrono::milliseconds(_sleepval));
			this_fiber::yield();
		}
	}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	int interval{100}, fcnt{-1}, sleepval{50};
	bool lorder{true}, skip{true};
	fiber_monitor::sort_mode sm{ fiber_monitor::sort_mode::by_ms };

	static constexpr auto optstr { "f:i:s:om:rkh" };
#ifdef _GNU_SOURCE
   static constexpr const std::array<option, 9> long_options
   {{
      { "help",		0, 0, 'h' },	{ "retain",		0, 0, 'r' },	{ "order",		0, 0, 'o' },
      { "noskip",		0, 0, 'k' },	{ "sleep",		1, 0, 's' },	{ "interval",	1, 0, 'i' },
      { "fibers",		1, 0, 'f' },	{ "mode",		1, 0, 'm' }
   }};

	for (int opt; (opt = getopt_long (argc, argv, optstr, long_options.data(), 0)) != -1;)
#else
	for (int opt; (opt = getopt (argc, argv, optstr)) != -1;)
#endif
	{
		try
		{
			switch (opt)
			{
			case 'f':
				fcnt = std::stoi(optarg);
				break;
			case 's':
				sleepval = std::stoi(optarg);
				break;
			case 'm':
				if (long unsigned ism{ std::stoul(optarg) - 1UL }; ism < static_cast<long unsigned>(fiber_monitor::sort_mode::count))
					sm = fiber_monitor::sort_mode(ism);
				break;
			case 'r':
				fibers::set_flag(global_fiber_flags::retain);
				break;
			case 'k':
				skip = false;
				break;
			case 'o':
				lorder = false;
				break;
			case 'i':
				interval = std::stoi(optarg);
				break;
			case '?':
			case ':':
			case 'h':
				std::cout << "Usage: " << argv[0] << " -[" << optstr << R"(]
  -i,--interval interval msecs (default 100)
  -f,--fibers fiber count (default )" << (fiber_monitor::get_terminal_dimensions().second - 4) << R"()
  -s,--sleep sleep msecs (default 50)
  -m,--mode sort mode )" << fiber_monitor::sort_help() << R"((default 3)
  -o,--order no launch order
  -k,--noskip no skip main
  -r,--interval retain finished fibers
  -h,--help)" << std::endl;
				exit(1);
			default:
				break;
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "exception: " << e.what() << std::endl;
			exit(1);
		}
	}

	if (skip)
		fibers::set_flag(global_fiber_flags::skipmain);
	fiber_monitor fm{std::chrono::milliseconds(interval), sm};
	foo bar(fm, sleepval);
	if (fcnt == -1)
		fcnt = fm.get_dimensions().second - 4;
	std::vector<fiber_ptr> fbs;
	for (int ii{}; ii < fcnt; ++ii)
	{
		fbs.emplace_back(make_fiber({.launch_order=lorder ? ii : 99,.stacksz=(fcnt * 4096lu) & ~0xff},
			&foo::func, &bar, 2 * (ii + 1)))->set_params("sub"s + std::to_string(ii));
	}

	while (fibers::has_fibers())
	{
		this_fiber::yield();
		if (!fm)
		{
			fibers::kill_all();
			break;
		}
	}
	fm.update();
	std::this_thread::sleep_for(1s);
	return 0;
}