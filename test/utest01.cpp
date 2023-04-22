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
#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <sstream>
#include <future>
#include <thread>
#include <queue>
#include <random>
#include <array>
#include <string_view>
#include <list>
#include <chrono>
#include <fix8/fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
int future_sub(int arg)
{
	return arg * 100;
}

TEST_CASE("Future - fiber async wth std::future", "[fiber][async][future]")
{
	std::future<int> myfuture { async(&future_sub, 10) };
	REQUIRE(myfuture.get() == 1000);
}

//-----------------------------------------------------------------------------------------
template<typename T>
void fibonacci (T& val)
{
	val = 0;
	for(T next{1}; !fibers::terminating(); val = std::exchange(next, val + next))
		this_fiber::yield();
}

TEST_CASE("Fibonacci - calculating first 18 fibonacci numbers", "[fiber]")
{
	uint64_t val;
	fiber(fibonacci<uint64_t>, std::ref(val)).detach();
	for (int num{18}; num--; this_fiber::yield())
		;
	REQUIRE(val == 1597);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("Promise - fiber with std::promise", "[fiber][promise][future]")
{
	std::promise<int> mypromise;
	auto myfuture { mypromise.get_future() };
	fiber sub_co([](int arg, std::promise<int>& pr)
	{
		for (int ii{}; ii++ < arg; this_fiber::yield())
			;
		pr.set_value(arg * 100);
	}, 10, std::ref(mypromise));

	while(sub_co)
		this_fiber::yield();
	REQUIRE(myfuture.get() == 1000);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("Task - with std::future and std::packaged_task", "[fiber][task]")
{
	std::packaged_task task([](int arg)
	{
		for (int ii{}; ii++ < arg; this_fiber::yield())
			;
		return arg * 100;
	});
	auto myfuture { task.get_future() };
	fiber myfiber(std::move(task), 10);
	while(myfiber)
		this_fiber::yield();
	REQUIRE(myfuture.get() == 1000);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("Wordset - fiber reconstructing sentence", "[fiber][launch_all_with_params_n]")
{
	static constexpr const std::array wordsets
	{
		std::array { R"("I )",		"all ",		"said ",		"It’s ",		"I’m ",			""					},
		std::array { "for ",			"who ",		"me. ",		"them ",		"myself.\"\n",	""					},
		std::array { "am ",			"of ",		"no ",		"because ",	"doing ",		" - Albert ",	},
		std::array { "thankful ",	"those ",	"to ",		"of ",		"it ",			"Einstein\n"	},
	};
	static const std::string_view cmpstr { R"("I am thankful for all of those who said no to me. It’s because of them I’m doing it myself."
 - Albert Einstein
)"};

	std::ostringstream ostr;
	const auto func([&ostr](const auto& words)
	{
		for (auto pp : words)
		{
			ostr << pp;
			this_fiber::yield();
		}
	});

	std::list<fiber> sts;
	fibers::set_flag(global_fiber_flags::skipmain);

	launch_all_with_params_n
	(
	 	sts,
		fiber_params{.launch_order=0}, std::bind(func, wordsets[0]),
		fiber_params{.launch_order=3}, std::bind(func, wordsets[1]),
		fiber_params{.launch_order=1}, std::bind(func, wordsets[2]),
		fiber_params{.launch_order=2}, std::bind(func, wordsets[3])
	);

	REQUIRE(ostr.str() == cmpstr);
}

//-----------------------------------------------------------------------------------------
void resume_sub(int arg, int& res)
{
	for (int ii{}; ii++ < arg; this_fiber::resume_main())
		;
	res = arg;
}

TEST_CASE("Resume - fiber resume with another function", "[fiber][resume]")
{
	int ii{}, result;
	for (fiber myfiber(&resume_sub, 10, std::ref(result)); myfiber; ++ii)
	{
		if (ii == 9)
		{
			myfiber.resume_with(&resume_sub, 5, std::ref(result));
			while (myfiber)
				this_fiber::yield();
		}
		this_fiber::yield();
	}
	REQUIRE(result == 5);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("Manyfibers - test with 1000 fibers", "[fiber]")
{
	static int cnt{};
	std::thread([]
	{
		for (int ii{}; ii < 1000; ++ii)
		{
			fiber([](int arg)
			{
				for (int ii{}; ii < arg; ++ii)
					this_fiber::yield();
				++cnt;
			},
			ii + 1).detach();
		}
	}).join();

	REQUIRE(cnt == 1000);
}

//-----------------------------------------------------------------------------------------
class generator_foo
{
	std::queue<long> _queue;
   fiber _produce, _consume;

   void producer()
   {
		for (int ii{1}; ii <= 10; ++ii)
      {
			_queue.push(ii);
			if (ii % 2 == 0)
				_consume.resume(); // switch to consumer
      }
		_consume.schedule(); // consumer is next fiber to run
   }
   void consumer(long& res)
   {
		res = 1;
      while (_produce)
      {
			while(!_queue.empty())
			{
				res *= _queue.front();
				_queue.pop();
			}
			_produce.resume(); // switch to producer
      }
   }

public:
   generator_foo(long& res) : _produce(&generator_foo::producer, this), _consume(&generator_foo::consumer, this, std::ref(res))
	{
		_produce.resume(); // switch to producer
	}
};

TEST_CASE("Generator - generate 10!", "[fiber][generator]")
{
	long result{};
   generator_foo(std::ref(result));
	REQUIRE(result == 3628800);
}

//-----------------------------------------------------------------------------------------
TEST_CASE("jfiber - auto joining fiber", "[fiber]")
{
	REQUIRE_NOTHROW(std::thread([]()
	{
		jfiber {[]()
		{
			for(int ii{}; ii < 10; ++ii)
				std::this_thread::sleep_for(100ms);
		}};
	}).join());
}

