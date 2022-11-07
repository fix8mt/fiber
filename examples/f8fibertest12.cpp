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
	static constexpr const std::array<std::array<std::string_view, 6>, 4> wordset
	{{
		{	R"("I)",		"all",	"said",	"It’s",		"I’m",								},
		{	"thankful",	"those",	"to",		"of",			"it",				"Einstein\n"	},
		{	"am",			"of",		"no",		"because",	"doing",			"- Albert",		},
		{	"for",		"who",	"me.",	"them",		"myself.\"\n"						},
	}};

	static const auto func([](const auto& words)
	{
		for (const auto& pp : words)
		{
			if (pp.data())
			{
				if (static thread_local bool notfirst{}; std::exchange(notfirst, true))
					std::cout << ' ';
				std::cout << pp;
			}
			this_fiber::yield();
		}
	});

	std::thread([]()
	{
		launch_all // will print in fiber work order
		(
			std::bind(func, wordset[0]),
			std::bind(func, wordset[1]),
			std::bind(func, wordset[2]),
			std::bind(func, wordset[3])
		);
	}).join();

	std::thread([]()
	{
		launch_all_with_params // will print in specified order
		(
			fiber_params{.launch_order=1}, std::bind(func, wordset[0]),
			fiber_params{.launch_order=3}, std::bind(func, wordset[1]),
			fiber_params{.launch_order=2}, std::bind(func, wordset[2]),
			fiber_params{.launch_order=4}, std::bind(func, wordset[3])
		);
	}).join();

	return 0;
}
