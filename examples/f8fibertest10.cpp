#include <iostream>
#include <string_view>
#include <vector>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main()
{
	std::thread([]()
	{
		static const std::vector<std::vector<std::string_view>> wordset
		{
			{	"I",			"all",	"said",	"It’s",		"I’m",		"\n  –",		},
			{	"am",			"of",		"no",		"because",	"doing",		"Albert",	},
			{	"thankful",	"those",	"to",		"of",			"it",			"Einstein"	},
			{	"for",		"who",	"me.",	"them",		"myself.", 					},
		};
		int order{};
		for (const auto& pp : wordset)
		{
			f8_fiber ({.launch_order=order++}, [](const auto& words)
			{
				for (const auto& pp : words)
				{
					std::cout << pp << ' ';
					this_fiber::yield();
				}
			},pp).detach();
		}
		fibers::print();
	}).join();

	std::cout << std::endl;
	return 0;
}
