#include <iostream>
#include <functional>
#include <string_view>
#include <deque>
#include <set>
#include <thread>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::chrono_literals;

//-----------------------------------------------------------------------------------------
void sub(int arg)
{
	std::cout << "\tstarting " << this_fiber::name() << ' ' << arg << '\n';
	for (int ii{}; ii < arg; this_fiber::yield())
	{
		std::cout << '\t' << std::this_thread::get_id() << ' ' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
		std::this_thread::sleep_for(1s);
	}
	std::cout << "\tleaving " << this_fiber::name() << ' ' << arg << '\n';
}

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	int stcnt{20};
	try
	{
		if (argc > 1)
			stcnt = std::stoi(argv[1]);
	}
	catch (const std::exception& e)
	{
		std::cerr << "exception: " << e.what() << std::endl;
		exit(1);
	}

	fiber f0({.name="first"}, &sub, 15), f1({.name="second"}, &sub, 12), f2({.name="third"}, &sub, 13);
	std::thread t1([stcnt]()
	{
		fiber ft1({.name="tfirst"}, &sub, stcnt);
		for (int ii{}; fibers::has_fibers(); ++ii)
		{
			std::cout << "main1 " << ii << '\n';
			switch(ii)
			{
			case 4:
			case 9:
			case 11:
			case 21:
				fibers::print();
				break;
			default:
				break;
			}
			std::cout << "main1\n";
			this_fiber::yield();
		}
		std::cout << "Exiting from main1\n";
	});
	std::this_thread::sleep_for(250ms);
	try
	{
		for (int ii{}; fibers::has_fibers(); ++ii)
		{
			std::cout << "main " << ii << '\n';
			switch(ii)
			{
			case 5:
				std::cout << "transferring " << f0.get_id() << " from " << std::this_thread::get_id() << " to " << t1.get_id() << '\n';
				f0.move(t1.get_id());
				break;
			case 10:
				std::cout << "transferring " << f1.get_id() << " from " << std::this_thread::get_id() << " to " << t1.get_id() << '\n';
				f1.move(t1.get_id());
				break;
			case 4:
			case 9:
			case 11:
				fibers::print();
				[[fallthrough]];
			default:
				this_fiber::yield();
				break;
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "exception " << e.what() << '\n';
		std::cerr << "killed " << fibers::kill_all() << " fibers\n";
	}
	std::cout << "Exiting from main\n";
	t1.join();
	return 0;
}
