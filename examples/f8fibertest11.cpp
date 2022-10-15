#include <iostream>
#include <string_view>
#include <array>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main()
{
	auto func([](const auto& words)
	{
		for (const auto& pp : words)
		{
			std::cout << pp << ' ';
			this_fiber::yield();
		}
	});

	static const std::vector<std::vector<std::string_view>> wordset
	{
		{	"I",			"all",	"said",	"It’s",		"I’m",		"\n  –",			},
		{	"thankful",	"those",	"to",		"of",			"it",			"Einstein\n"	},
		{	"for",		"who",	"me.",	"them",		"myself.", 						},
		{	"am",			"of",		"no",		"because",	"doing",		"Albert",		},
	};
	std::array work
	{
		fiber{ {.launch_order=0,.name="first"},	func, wordset[0] },
		fiber{ {.launch_order=2,.name="second"},	func, wordset[1] },
		fiber{ {.launch_order=3,.name="third"},	func, wordset[2] },
		fiber{ {.launch_order=1,.name="fourth"},	func, wordset[3] }
	};

	fibers::print();
	std::cout << std::endl;

	while(fibers::has_fibers())
		this_fiber::yield();

	return 0;
}
