#include <iostream>
#include <thread>
#include <cstdlib>
#include <fix8/f8fiber.hpp>

using namespace FIX8;

//-----------------------------------------------------------------------------------------
// from Dilawar's Blog
// https://dilawar.github.io/posts/2021/2021-11-14-example-boost-fiber/
//-----------------------------------------------------------------------------------------
int main()
{
	static int ii{};

	fiber([]()
	{
		do
		{
			std::cout << 'a';
			this_fiber::yield();
		}
		while (++ii < 20);
	}).detach();

	fiber([]()
	{
		do
		{
			std::cout << 'b';
			std::thread([]() { std::cout << 'B'; }).detach();
			this_fiber::yield();
		}
		while (++ii < 20);
	}).detach();

	std::cout << 'X';
	return 0;
}

// XababBabBabBababBBabBabBabBabBB
// XababBabBabBabBabBabBabaBbBabBB
// XababBabBabBabBabBabBabBabBabBB
