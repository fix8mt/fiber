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
#include <fix8/f8fiber.hpp>
#include <fix8/f8fibermonitor.hpp>
#include <unistd.h>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
class foo
{
	fiber_monitor& _fm;
	int _sleepval;
	window_frame _xyxy;

public:
	foo(fiber_monitor& fm, int sleepval, bool first) : _fm(fm), _sleepval(sleepval)
	{
		auto [x, y] { _fm.get_dimensions() };
		_xyxy = first ? window_frame({{}, {x, y / 2 - 1}}) : window_frame({{0, y / 2}, {x, y}});
	}

	void func(int arg)
	{
		for (int ii{}; ii < arg; ++ii)
		{
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
	int interval{100}, sleepval{50};
	bool lorder(true);
	static constexpr const char *optstr{"i:s:orh"};
	for (int opt; (opt = getopt(argc, argv, optstr)) != -1;)
	{
		try
		{
			switch (opt)
			{
			case 's':
				sleepval = std::stoi(optarg);
				break;
			case 'o':
				lorder = false;
				break;
			case 'r':
				fibers::set_flag(global_fiber_flags::retain);
				break;
			case 'i':
				interval = std::stoi(optarg);
				break;
			case ':': case '?':
				std::cout << '\n';
				[[fallthrough]];
			case 'h':
				std::cout << "Usage: " << argv[0] << " [-" << optstr << "]" << R"(
  -i interval msecs (default 100)
  -r retain finished fibers
  -s sleep msecs (default 50)
  -o no launch order
  -h help)" << std::endl;
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

	fiber_monitor fm{std::chrono::milliseconds(interval)};
	std::thread::id id0, id1;
	auto func([&](std::thread::id& oid, bool first=false)
	{
		foo bar(fm, sleepval, first);
		int fcnt { fm.get_dimensions().second / 2 - 6 };
		//int fcnt { fm.get_dimensions().second / 2 - 3 };
		std::vector<std::unique_ptr<fiber>> fbs;
		for (int ii{}; ii < fcnt; ++ii)
		{
			std::ostringstream ostr;
			ostr << (first ? 'A' : 'B') << std::setfill('0') << std::setw(2) << ii;
			fbs.emplace_back(std::make_unique<fiber>(fiber_params{.launch_order=lorder ? ii : 99,.stacksz=8192},
				&foo::func, &bar, 5 * (1 + (ii % 8))))->set_params(ostr.str());
		}
		std::mt19937_64 rnde {std::random_device{}()};
		for (int ii{}; fibers::has_fibers(); ++ii)
		{
			if (!fm)
			{
				fibers::kill_all();
				break;
			}
			else if (std::uniform_int_distribution<int>(0, 5)(rnde) == 1)
			{
				auto& cvs { fibers::const_get_vars() };
				while(cvs.size())
				{
					if (auto ptr { cvs[std::uniform_int_distribution<int>(0, cvs.size() - 1)(rnde)] }; ptr && !ptr->is_main())
					{
						fibers::move(oid, ptr);
						break;
					}
				}
			}
			else
				this_fiber::yield();
		}
	});
	std::thread t1(func, std::ref(id1), true), t2(std::bind(func, std::ref(id0)));
	id0 = t1.get_id();
	id1 = t2.get_id();
	t1.join();
	t2.join();

	fm.update();
	std::this_thread::sleep_for(1s);
	return 0;
}
