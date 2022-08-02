//-----------------------------------------------------------------------------------------
// f8_fiber (header only) based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <array>
#include <string>
#include <random>

#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
class Reader : public f8_fiber
{
	static const size_t max_msg_len { 256 }, tlen { 10 };
	std::mt19937_64 rnd_engine { std::random_device{}() };
	int _rdcnt{};

	int get_bytes(char *where, int cnt)
	{
		const int tg { std::uniform_int_distribution<int>(0, cnt)(rnd_engine) };
		for (int ii{}; ii < tg; ++ii, ++where)
		{
			do
				*where = std::uniform_int_distribution<char>{48, 122}(rnd_engine);
			while(!std::isalnum(*where));
		}
		return tg;
	}

	static f8_fiber process (f8_fiber&& f, std::string& msg)
	{
		std::cout << std::setw(3) << msg.size() << ' ' << msg << '\n';
		return std::move(f); // jump back to read_nb
	}

	f8_fiber read_nb (f8_fiber&& f, std::string& msg)
	{
		std::array<char, max_msg_len> buff;
		for (int ii{}; ii < _rdcnt; ++ii)
		{
			auto hlen { std::uniform_int_distribution<size_t>{12, 16}(rnd_engine) }; // variable header length
			auto mlen { std::uniform_int_distribution<size_t>{80, 102}(rnd_engine) }; // variable body length
			// read header, yield if nothing to read
			for (int sofar{}; (sofar += get_bytes(buff.data() + sofar, hlen - sofar)) < hlen; f8_yield(f));
			buff[hlen++] = ':';
			// read body, yield if nothing to read
			for (int sofar{}; (sofar += get_bytes(buff.data() + hlen + sofar, mlen - sofar)) < mlen; f8_yield(f));
			buff[hlen + mlen++] = ':';
			// read trailer, yield if nothing to read
			for (int sofar{}; (sofar += get_bytes(buff.data() + hlen + sofar + mlen, tlen - sofar)) < tlen; f8_yield(f));
			msg.assign(buff.data(), hlen + mlen + tlen);
			f8_yield_with(f, &Reader::process, std::placeholders::_1, std::ref(msg)); // jump to process
		}
		std::cout << "read " << _rdcnt << " messages\n";
		return std::move(f);
	}

public:
	Reader(std::string& msg, int rdcnt)
		: _rdcnt(rdcnt), f8_fiber(&Reader::read_nb, this, std::placeholders::_1, std::ref(msg)) {}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	std::string msg;
	Reader reader(msg, argc > 1 ? std::stol(argv[1]) : 100);
	std::cout << reader << '\n';

	while (reader)
		f8_yield(reader);

	std::cout << "main exit\n";
	return 0;
}

