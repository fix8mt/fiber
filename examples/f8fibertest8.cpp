#include <iostream>
#include <string>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct foo
{
	int _a;
	foo(int a) : _a(a) { std::cout << "foo(" << _a << ",&_a=" << &_a << ")\n"; }
	~foo() { std::cout << "~foo(" << _a << ")\n"; }
};

//-----------------------------------------------------------------------------------------
void sub(int arg, int spacer)
{
	foo a(arg);
	const auto tabs { std::string(spacer, '\t') };
	std::cout << tabs << "starting " << this_fiber::name() << '\n';
	for (int ii{}; ii < arg; )
	{
		std::cout << tabs << this_fiber::name() << ": " << ++ii << '\n';
		this_fiber::yield();
	}
	std::cout << tabs << "leaving " << this_fiber::name() << '\n';
}

//-----------------------------------------------------------------------------------------
int main()
{
	int ii{};
	for (fiber myfiber({.name="sub"}, &sub, 10, 1); myfiber; this_fiber::yield())
	{
		std::cout << "main: " << ++ii << '\n';
		if (ii == 9)
		{
			myfiber.set_params("sub1").resume_with(&sub, 5, 2);
			for (int jj{}; myfiber; this_fiber::yield())
				std::cout << "main1: " << ++jj << '\n';
		}
	}
	std::cout << "Exiting from main\n";
	return 0;
}
