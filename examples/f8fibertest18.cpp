#include <iostream>
#include <future>
#include <random>
#include <list>
#include <string>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
int main(void)
{
	std::cout << "Starting main\n";
	std::list<std::future<int>> flist;

	std::thread([&flist]()
	{
		std::mt19937_64 rnde {std::random_device{}()};
		std::uniform_int_distribution<int> pgen {1, 1000000};

		for (int ii{1}; ii <= 10; ++ii)
		{
			std::packaged_task<int(int)> task([&rnde,&pgen](int arg)
			{
				std::cout << "\tStarting " << this_fiber::name() << '\n';
				int result{};
				for (int ii{1}; ii <= arg; this_fiber::yield())
				{
					result += pgen(rnde);
					std::cout << "\t\t" << this_fiber::name() << ": " << ii++ << ' ' << result << '\n';
				}
				std::cout << "\tLeaving " << this_fiber::name() << '\n';
				return result;
			});
			flist.emplace_back(task.get_future());
			fiber({.launch_order=ii,.stacksz=16384}, std::move(task), ii).set_params(("sub"s + std::to_string(ii)).c_str()).detach();
		}
		fibers::print();
	}).join();

	int result{}, kk{};
	for (auto& pp : flist)
	{
		auto val { pp.get() };
		result += val;
		std::cout << "got " << ++kk << " (" << val << ")\n";
	}
	std::cout << "result = " << result << '\n';
	std::cout << "Exiting main\n";
	return 0;
}
