#include <iostream>
#include <functional>
#include <deque>
#include <set>
#include <future>
#include <list>
#include <random>
#include <string>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
int main(void)
{
	std::cout << "Starting main\n";
	std::mt19937_64 rnde {std::random_device{}()};
	std::uniform_int_distribution<int> agen {1, 1000}, pgen {1, 1000000};

	for (int ii{}; ii < 10; ++ii)
	{
		fiber([&rnde,&pgen](int arg)
		{
			std::cout << "\tStarting " << this_fiber::name() << ' ' << arg << '\n';
			for (int ii{}; ii < arg; this_fiber::yield())
				std::cout << "\t\t" << this_fiber::name() << ": " << ++ii << ' ' << pgen(rnde) << '\n';
			std::cout << "\tLeaving " << this_fiber::name() << '\n';
		},
		agen(rnde)).set_params(("sub"s + std::to_string(ii)).c_str(), ii).detach();
	}
	fibers::print();
	std::cout << "Exiting main\n";
	return 0;
}
