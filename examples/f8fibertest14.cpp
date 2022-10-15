#include <iostream>
#include <string_view>
#include <functional>
#include <deque>
#include <set>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
struct blah { ~blah() { std::cout << "~blah(): " << this_fiber::name() << '\n'; } };

void doit(int arg, std::string_view spacer)
{
	blah b;
	std::cout << spacer << "starting " << this_fiber::name() << ' ' << arg << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
		std::cout << spacer << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
	std::cout << spacer << "leaving " << this_fiber::name() << ' ' << arg << '\n';
}

void doit_with_stoprequest(std::string_view spacer, bool& stop_requested)
{
	blah b;
	std::cout << spacer << "starting " << this_fiber::name() << '\n';
	for(int ii{};; this_fiber::yield())
	{
		std::cout << spacer << this_fiber::name() << ": " << ++ii << '\n';
		if (stop_requested)
		{
			std::cout << spacer << this_fiber::name() << ": stop requested\n";
			break;
		}
	}
	std::cout << spacer << "leaving " << this_fiber::name() << '\n';
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	fiber sub_co({.name="sub0"}, &doit, 9, "\t"), sub_co1({.name="sub1"}, [](int arg, std::string_view spacer)
	{
		blah b;
		bool stop_requested{};
		std::cout << spacer << "starting " << this_fiber::name() << ' ' << arg << '\n';
		fiber sub_co2({.name="sub1/sub",.join=true}, &doit_with_stoprequest, "\t\t", std::ref(stop_requested));
		for (int ii{}; ii < arg; this_fiber::yield())
			std::cout << spacer << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
		stop_requested = true;
		std::cout << spacer << "leaving " << this_fiber::name() << ' ' << arg << '\n';
	}, 10, "\t");
	fibers::print();
	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		std::cout << "main: " << std::dec << ii << '\n';
		this_fiber::yield();
		if (ii == 1)
			fibers::print();
	}
	std::cout << "Exiting from main\n";
	return 0;
}
