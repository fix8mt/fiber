#include <iostream>
#include <thread>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::chrono_literals;

//-----------------------------------------------------------------------------------------
void sub(int arg)
{
	std::cout << "\tstarting " << this_fiber::name() << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
	{
		std::cout << '\t' << std::this_thread::get_id() << ' ' << this_fiber::name() << ' ' << ++ii << '\n';
		std::this_thread::sleep_for(250ms);
	}
	std::cout << "\tleaving " << this_fiber::name() << '\n';
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	fiber f0({.name="first"}, &sub, 10), f2({.name="second"}, &sub, 10);
	std::thread t1([]()
	{
		jfiber ft1({.name="thread:first"}, &sub, 10);
		for (int ii{}; fibers::has_fibers(); ++ii)
		{
			std::cout << "main1 " << ii << '\n';
			switch(ii)
			{
			case 1:
			case 9:
				fibers::print();
				[[fallthrough]];
			default:
				std::this_thread::sleep_for(250ms);
				this_fiber::yield();
				break;
			}
		}
		std::cout << std::this_thread::get_id() << " Exiting from main1\n";
	});

	std::this_thread::sleep_for(100ms);

	for (int ii{}; fibers::has_fibers(); ++ii)
	{
		std::cout << "main " << ii << '\n';
		switch(ii)
		{
		case 5:
			std::cout << "transferring " << f0.get_id() << " from "
				<< std::this_thread::get_id() << " to " << t1.get_id() << '\n';
			f0.move(t1.get_id());
			break;
		case 4:
		case 9:
			fibers::print();
			[[fallthrough]];
		default:
			this_fiber::yield();
			break;
		}
	}
	std::cout << "waiting at join...\n";
	fibers::print();
	t1.join();
	std::cout << std::this_thread::get_id() << " Exiting from main\n";
	return 0;
}
