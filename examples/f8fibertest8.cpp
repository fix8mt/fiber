#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <string>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
struct foo
{
	void sub(int arg, int spacer)
	{
		std::cout << "\tstarting " << arg << ' ' << this_fiber::name() << '\n';
		for (int ii{}; ii < arg; )
		{
			std::cout << std::string(spacer, '\t') << arg << ": " << ++ii << '\n';
			this_fiber::yield();
		}
		std::cout << "\tleaving " << arg << '\n';
	}
};

//-----------------------------------------------------------------------------------------
int main(void)
{
	foo bar;
	const int limit{10};
	fiber sub_co({.name="sub"}, &foo::sub, &bar, limit, 1);
	for (int ii{}; sub_co; )
	{
		std::cout << "main: " << ++ii << '\n';
		if (ii == limit - 1)
		{
			sub_co.resume_with(&foo::sub, &bar, 5, 2);
			for (int jj{}; sub_co; )
			{
				std::cout << "main1: " << ++jj << '\n';
				this_fiber::yield();
			}
		}
		this_fiber::yield();
	}
	std::cout << "Exiting from main\n";
	return 0;
}
