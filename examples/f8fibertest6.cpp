//-----------------------------------------------------------------------------------------
// f8_fiber (header only) based on boost::fiber, x86_64 / linux only / de-boosted
// Modifications Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//-----------------------------------------------------------------------------------------
#include <iostream>
#include <iomanip>
#include <thread>
#include <cctype>
#include <array>
#include <string>
#include <random>
#include <exception>

#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
struct ReaderHelper
{
	std::exception_ptr eptr;
	std::string msg;
	int rdcnt;
	bool was_exception() const { return static_cast<bool>(eptr); }
};

//-----------------------------------------------------------------------------------------
class Reader : public f8_fiber
{
	static constexpr size_t _max_msg_len {256}, _tlen {10};
	std::mt19937_64 _rnd_engine { std::random_device{}() };
	std::uniform_int_distribution<char> _chd{0, 63};
	std::uniform_int_distribution<size_t> _hld{12, 16}, _mld{80, 102};
	ReaderHelper& _obj;

	int read_bytes(char *where, int cnt) // simulate a read that doesn't guarentee to read all that is asked
	{
		static constexpr const char *b64set{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
		// 50% will not read all requested
		if (std::bernoulli_distribution()(_rnd_engine))
			cnt = std::uniform_int_distribution<int>{0, cnt - 1}(_rnd_engine);
		for (const auto ptr { where + cnt }; where < ptr; ++where)
			*where = b64set[_chd(_rnd_engine)];
		if (std::bernoulli_distribution(.005)(_rnd_engine)) // 0.5% chance of a read exception
			throw std::runtime_error("exception reading: " + std::to_string(cnt));
		return cnt;
	}

	static void process (std::string& msg)
	{
		std::cout << std::setw(3) << msg.size() << ' ' << msg << '\n';
	}

	f8_fiber read_nb (f8_fiber&& f)
	{
		try
		{
			std::array<char, _max_msg_len> buff;
			for (int ii{}; ii < _obj.rdcnt; ++ii)
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

				_obj.msg.assign(buff.data(), hlen + mlen + _tlen);
				process(_obj.msg);
			}
			std::cout << "read " << _obj.rdcnt << " messages\n";
		}
		catch (...)
		{
			_obj.eptr = std::current_exception();
		}
		return std::move(f);
	}

public:
	Reader(ReaderHelper& obj) : _obj(obj), f8_fiber(std::allocator_arg, f8_fixedsize_heap_stack(), &Reader::read_nb, this, std::placeholders::_1) {}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	try
	{
		ReaderHelper obj { .rdcnt = argc > 1 ? std::stoi(argv[1]) : 100 };
		Reader reader(obj);
		int yields{};

		while (reader)
		{
			f8_yield(reader);
			try
			{
				if (obj.was_exception())
					std::rethrow_exception(obj.eptr);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Reader: " << e.what() << '\n';
			}
			catch (...)
			{
				std::cerr << "Reader: unknown exception\n";
			}
			++yields;
		}

		std::cout << yields << " yields\nmain exit\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "main: " << e.what() << '\n';
	}
	return 0;
}

