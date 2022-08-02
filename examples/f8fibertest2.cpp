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
using Message = std::pair<bool, std::string>;

//-----------------------------------------------------------------------------------------
class Reader
{
	static const size_t max_msg_len { 256 }, header_len { 16 }, trailer_len { 10 };
	std::mt19937_64 rnd_engine { std::random_device{}() };

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

public:
	Reader() = default;

	FIX8::f8_fiber read_nb (FIX8::f8_fiber&& f, Message& msg, int cnt)
	{
		std::array<char, max_msg_len> buff;
		for (int ii{}; ii < cnt; ++ii)
		{
			const auto mlen { std::uniform_int_distribution<size_t>{80, 102}(rnd_engine) };
			for (int sofar{}; (sofar += get_bytes(buff.data() + sofar, header_len - sofar)) < header_len; f8_yield(f));
			for (int sofar{}; (sofar += get_bytes(buff.data() + header_len + sofar, mlen - sofar)) < mlen; f8_yield(f));
			for (int sofar{}; (sofar += get_bytes(buff.data() + header_len + sofar + mlen, trailer_len - sofar)) < trailer_len; f8_yield(f));
			msg = {true, {buff.data(), header_len + mlen + trailer_len}};
			f8_yield(f);
		}
		std::cout << "read " << cnt << " messages\n";
		return std::move(f);
	}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	Reader reader;
	Message msg{};
	auto& [ready, str] { msg };
	f8_fiber f0(std::bind(&Reader::read_nb, &reader, std::placeholders::_1, std::ref(msg), argc > 1 ? std::stol(argv[1]) : 100));

	for (int pauses{}; f0;)
	{
		f8_yield(f0);
		if (ready)
		{
			std::cout << std::setw(2) << pauses << " => " << std::setw(3) << str.size() << ' ' << str << '\n';
			ready = false;
			pauses = 0;
		}
		else
			++pauses;
	}

	return 0;
}

