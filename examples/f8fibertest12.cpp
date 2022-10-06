#include <iostream>
#include <string_view>
#include <array>
#include <utility>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main()
{
	static const std::vector<std::vector<std::string_view>> wordset
	{
		{	"I",			"all",	"said",	"It’s",		"I’m",		"\n  –",			},
		{	"thankful",	"those",	"to",		"of",			"it",			"Einstein\n"	},
		{	"am",			"of",		"no",		"because",	"doing",		"Albert",		},
		{	"for",		"who",	"me.",	"them",		"myself.", 						},
	};

	std::thread([]()
	{
		auto func([](const auto& words)
		{
			for (const auto& pp : words)
			{
				if (static bool first{}; std::exchange(first, true))
					std::cout << ' ';
				std::cout << pp;
				this_fiber::yield();
			}
		});
		launch_all_with_params
		(
			 f8_fiber_params{.launch_order=0,.name="first"}, std::bind(func, wordset[0]),
			 f8_fiber_params{.launch_order=2,.name="second"}, std::bind(func, wordset[1]),
			 f8_fiber_params{.launch_order=1,.name="third"}, std::bind(func, wordset[2]),
			 f8_fiber_params{.launch_order=3,.name="fourth"}, std::bind(func, wordset[3])
		);
		/*
		launch_all
		(
			 std::bind(func, wordset[0]),
			 std::bind(func, wordset[1]),
			 std::bind(func, wordset[2]),
			 std::bind(func, wordset[3])
		);
		*/
		fibers::print();
		std::cout << std::endl;
	}).join();

	return 0;
}
