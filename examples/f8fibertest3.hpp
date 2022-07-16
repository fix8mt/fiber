//-----------------------------------------------------------------------------------------
// f8_fiber (header only) based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//-----------------------------------------------------------------------------------------
#include <fix8/f8fiber.hpp>

#ifndef fooclass
#define fooclass
//-----------------------------------------------------------------------------------------
class foo
{
	int _cnt{};

public:
	foo(int cnt) : _cnt(cnt) {}

	FIX8::f8_fiber func (FIX8::f8_fiber&& f, bool& flags);
};
#endif

