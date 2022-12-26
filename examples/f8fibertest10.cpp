#include <iostream>
#include <string_view>
#include <array>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
int main()
{
	std::thread([]()
	{
		static constexpr const std::array<std::array<std::string_view, 6>, 4> wordset
		{{
			{	R"("I)",		"all",	"said",	"It’s",		"I’m",		"\n –",		},
			{	"am",			"of",		"no",		"because",	"doing",		"Albert",	},
			{	"thankful",	"those",	"to",		"of",			"it",			"Einstein"	},
			{	"for",		"who",	"me.",	"them",		R"(myself.")",				},
		}};
		for (const auto& pp : wordset)
		{
			fiber ([](const auto& words)
			{
				for (const auto& qq : words)
				{
					std::cout << qq << ' ';
					this_fiber::yield();
				}
			}, pp).detach();
		}
	}).join();

	std::cout << std::endl;
	return 0;
}
