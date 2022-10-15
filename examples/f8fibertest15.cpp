#include <iostream>
#include <string_view>
#include <thread>
#include <chrono>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
class blah
{
	std::string_view tabs;
public:
	blah() = default;
	blah(std::string_view tb) : tabs(std::move(tb)) {}
	~blah() { std::cout << tabs << "~blah(): " << this_fiber::name() << '\n'; }
};

void doit_with_stoprequest(bool& stop_requested)
{
	blah b("\t");
	bool waitagain{};
	std::cout << '\t' << "Starting " << this_fiber::name() << '\n';
	for(int ii{};; this_fiber::yield())
	{
		std::cout << '\t' << this_fiber::name() << ": " << ++ii << '\n';
		if (stop_requested)
		{
			if (waitagain)
				break;
			else
			{
				std::cout << '\t' << this_fiber::name() << ": stop requested\n";
				waitagain = true;
			}
		}
	}
	std::cout << '\t' << "Leaving " << this_fiber::name() << '\n';
}

//-----------------------------------------------------------------------------------------
int main(void)
{
	std::cout << "Starting " << this_fiber::name() << '\n';
	blah b;
	bool stop_requested{};
	fiber sub_co({.name="sub",.join=true}, &doit_with_stoprequest, std::ref(stop_requested));
	for (int ii{}; ii < 5; this_fiber::yield())
	{
		std::cout << this_fiber::name() << ": " << ++ii << '\n';
		std::this_thread::sleep_for(100ms);
		if (ii == 5)
			stop_requested = true;
	}
	fibers::print();
	std::cout << "Exiting " << this_fiber::name() << '\n';
	return 0;
}
