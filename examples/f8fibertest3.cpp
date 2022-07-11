//-----------------------------------------------------------------------------------------
// f8_fiber based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <string>

#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
class foo
{
	int _cnt{};

public:
	foo(int cnt) : _cnt(cnt) {}

	f8_fiber func (f8_fiber&& f, bool& flags)
	{
		std::cout << '\t' << "func:entry\n";
		std::cout << '\t' << "caller id:" << f.get_id() << '\n';
		for (int kk{}; kk < _cnt; ++kk)
		{
			std::cout << '\t' << "func:" << kk << '\n';
			f.resume(f);
			std::cout << '\t' << "func:" << kk << " (resumed)\n";
		}
		flags = true;
		std::cout << '\t' << "func:exit\n";
		return std::move(f);
	}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	bool flags{};
	foo bar(argc > 1 ? std::stol(argv[1]) : 5);
	f8_fiber f0(std::bind(&foo::func, &bar, std::placeholders::_1, std::ref(flags)));
	std::cout << "fiber id:" << f0.get_id() << '\n';
	std::cout << "flags=" << std::boolalpha << flags << '\n';

	for (int ii{}; f0; ++ii)
	{
		std::cout << "main:" << ii << '\n';
		f0.resume(f0);
		std::cout << "main:" << ii << " (resumed)\n";
	}
	std::cout << "flags=" << std::boolalpha << flags << '\n';
	std::cout << "main:exit\n";
	return 0;
}

