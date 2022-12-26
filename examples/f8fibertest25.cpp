#include <iostream>
#include <functional>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std::literals;

//-----------------------------------------------------------------------------------------
void doit()
{
	std::cout << "\tstarting " << this_fiber::name() << '\n';
	this_fiber::yield();
	for(int ii{}; ii < 10; std::this_thread::sleep_for(100ms))
		std::cout << '\t' << this_fiber::name() << ": " << ++ii << '\n';
	std::cout << "\tleaving " << this_fiber::name() << '\n';
}

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	std::unique_ptr<fiber> fb { argc > 1 ? new fiber({.name="fiber"}, &doit)
													 : new jfiber({.name="jfiber"}, &doit) };
	this_fiber::yield();
	fibers::print();
	std::cout << "Exiting from main\n";
	return 0;
}
