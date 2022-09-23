//-----------------------------------------------------------------------------------------
// f8_fiber (header only) based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <functional>

#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct Test
{
	f8_fiber foo(f8_fiber&& f, int i)
	{
		std::cout << "foo:entry\n";
		for (int kk{}; kk < i; ++kk)
		{
			std::cout << "\tfoo:" << kk << '\n';
			f.resume(f);
			std::cout << "\tfoo resumed:" << kk << '\n';
		}
		std::cout << "foo:exit\n";
		return std::move(f);
	}
};

//-----------------------------------------------------------------------------------------
f8_fiber bar1(f8_fiber&& f, int i)
{
	std::cout << "bar1:entry\n";
	for (int kk{}; kk < i; ++kk)
	{
		std::cout << "\tbar1:" << kk << '\n';
		f.resume(f);
		std::cout << "\tbar1 resumed:" << kk << '\n';
	}
	std::cout << "bar1:exit\n";
	return std::move(f);
}

//-----------------------------------------------------------------------------------------
f8_fiber bar0(f8_fiber&& f)
{
	std::cout << "bar0:entry\n";
	for (int kk{}; kk < 7; ++kk)
	{
		std::cout << "\tbar0:" << kk << '\n';
		f.resume(f);
		std::cout << "\tbar0 resumed:" << kk << '\n';
	}
	std::cout << "bar0:exit\n";
	return std::move(f);
}

//-----------------------------------------------------------------------------------------
int main()
{
	Test test;
	f8_fiber f0 { &bar0 };
	f8_fiber f1 { std::bind(&Test::foo, &test, std::placeholders::_1, 4) };
	f8_fiber f2 { std::bind(&bar1, std::placeholders::_1, 12) };
	f8_fiber f3 { std::bind([](f8_fiber&& f, int i)
	{
		std::cout << "bar2:entry\n";
		for (int kk{}; kk < i; ++kk)
		{
			std::cout << "\tbar2:" << kk << '\n';
			f.resume(f);
			std::cout << "\tbar2 resumed:" << kk << '\n';
		}
		std::cout << "bar2:exit\n";
		return std::move(f);
	}, std::placeholders::_1, 9) };

	for (int ii{}; f0 || f1 || f2 || f3; ++ii)
	{
		std::cout << "main:" << ii << '\n';
		f0.resume(f0);
		f1.resume(f1);
		f2.resume(f2);
		f3.resume(f3);
	}
	return 0;
}

