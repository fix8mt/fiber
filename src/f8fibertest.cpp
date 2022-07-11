//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include <fix8/f8fiber.hpp>
using namespace FIX8;
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
using namespace std::chrono_literals;
struct X
{
	f8_fiber foo(f8_fiber&& f, int i)
	{
		std::cout << "foo:entry" << std::endl;
		for (int kk{}; kk < i; ++kk)
		{
			std::cout << "\tfoo:" << kk << std::endl;
			std::this_thread::sleep_for(10ms);
			f = std::move(f).resume();
			std::cout << "\tfoo returned:" << kk << std::endl;
		}
		std::cout << "foo:exit\n";
		return std::move(f);
	}
};

f8_fiber bar(f8_fiber&& f, int i)
{
	std::cout << "bar1:entry" << std::endl;
	for (int kk{}; kk < i; ++kk)
	{
		std::cout << "\tbar1:" << kk << std::endl;
		std::this_thread::sleep_for(10ms);
		f = std::move(f).resume();
		std::cout << "\tbar1 returned:" << kk << std::endl;
	}
	std::cout << "bar1:exit\n";
	return std::move(f);
}

f8_fiber bar3(f8_fiber&& f)
{
	std::cout << "bar0:entry" << std::endl;
	for (int kk{}; kk < 7; ++kk)
	{
		std::cout << "\tbar0:" << kk << std::endl;
		std::this_thread::sleep_for(10ms);
		f = std::move(f).resume();
		std::cout << "\tbar0 returned:" << kk << std::endl;
	}
	std::cout << "bar0:exit\n";
	return std::move(f);
}

int main()
{
	X x;
	f8_fiber f0 { &bar3, 8 * 1024 * 1024 };
	f8_fiber f1 { std::bind(&X::foo, x, std::placeholders::_1, 7) };
	f8_fiber f2 { std::bind(&bar, std::placeholders::_1, 8), 8 * 1024 * 1024 };
	f8_fiber f3 { std::bind([](f8_fiber&& f, int i)
	{
		std::cout << "bar2:entry" << std::endl;
		for (int kk{}; kk < i; ++kk)
		{
			std::cout << "\tbar2:" << kk << std::endl;
			std::this_thread::sleep_for(10ms);
			f = std::move(f).resume();
			std::cout << "\tbar2 returned:" << kk << std::endl;
		}
		std::cout << "bar2:exit\n";
		return std::move(f);
	}, std::placeholders::_1, 7), 2 * 1024 * 1024 };

	for (int ii{}; f0 || f1 || f2 || f3; ++ii)
	{
		std::cout << "main:" << ii << std::endl;
		f0.resume(f0);
		f1.resume(f1);
		f2.resume(f2);
		f3.resume(f3);
	}
	return 0;
}

