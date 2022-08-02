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
class Reader : public f8_fiber
{
	static const size_t _max_msg_len { 256 }, _hlen { 16 }, _tlen { 10 };
	std::mt19937_64 _rnd_engine { std::random_device{}() };
	std::uniform_int_distribution<char> _chd{0, 63};
	std::uniform_int_distribution<size_t> _hld{12, 16}, _mld{80, 102};
	int _rdcnt{};

	int read_bytes(char *where, int cnt)
	{
		static constexpr const char *b64set{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
		if (std::bernoulli_distribution()(_rnd_engine))
			cnt = std::uniform_int_distribution<int>{0, cnt - 1}(_rnd_engine);
		for (const auto ptr { where + cnt }; where < ptr; ++where)
			*where = b64set[_chd(_rnd_engine)];
		return cnt;
	}

	f8_fiber read_nb (f8_fiber&& f, Message& msg)
	{
		std::array<char, _max_msg_len> buff;
		for (int ii{}; ii < _rdcnt; ++ii)
		{
			auto hlen { _hld(_rnd_engine) }, mlen { _mld(_rnd_engine) }; // variable header and body length
			// read header, yield if nothing or insufficent to read
			for (int sofar{}; (sofar += read_bytes(buff.data() + sofar, hlen - sofar)) < hlen; f8_yield(f));
			buff[hlen++] = ':'; // separator
			// read body, yield if nothing or insufficent to read
			for (int sofar{}; (sofar += read_bytes(buff.data() + hlen + sofar, mlen - sofar)) < mlen; f8_yield(f));
			buff[hlen + mlen++] = ':'; // separator
			// read trailer, yield if nothing or insufficent to read
			for (int sofar{}; (sofar += read_bytes(buff.data() + hlen + sofar + mlen, _tlen - sofar)) < _tlen; f8_yield(f));
			msg = { true, {buff.data(), hlen + mlen + _tlen} };
			f8_yield(f);
		}
		std::cout << "read " << _rdcnt << " messages\n";
		return std::move(f);
	}

public:
	Reader(Message& msg, int rdcnt)
		: _rdcnt(rdcnt), f8_fiber(&Reader::read_nb, this, std::placeholders::_1, std::ref(msg)) {}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	Message msg;
	auto& [ready, str]{ msg };
	Reader reader(msg, argc > 1 ? std::stol(argv[1]) : 100);

	for (int pauses{}; reader;)
	{
		f8_yield(reader);
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

