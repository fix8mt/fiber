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
class test_exception : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
	virtual ~test_exception() { std::cerr << "~test_exception()\n "; }
};

//-----------------------------------------------------------------------------------------
class Reader
{
	enum class NonBlockingReadResult { success, error, no_full_message_available };

	static constexpr size_t _max_msg_len {256}, _tlen {10};
	std::mt19937_64 _rnd_engine { std::random_device{}() };
	std::uniform_int_distribution<char> _chd{0, 63};
	std::uniform_int_distribution<size_t> _hld{12, 16}, _mld{80, 102};
	std::exception_ptr _eptr;
	std::string& _msg;
	int _invalid{}, _processed{};
	const int _rdcnt;

	int read_bytes(char *where, int cnt) // simulate a read that doesn't guarentee to read all that is asked
	{
		static constexpr const char *b64set{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
		// 50% will not read all requested
		if (std::bernoulli_distribution()(_rnd_engine))
			cnt = std::uniform_int_distribution<int>{0, cnt - 1}(_rnd_engine);
		for (const auto ptr { where + cnt }; where < ptr; ++where)
			*where = b64set[_chd(_rnd_engine)];
		if (std::bernoulli_distribution(.01)(_rnd_engine)) // 1% chance of a read exception
			throw test_exception("exception reading: " + std::to_string(cnt));
		return cnt;
	}

	int process (int cnt, std::string& msg)
	{
		std::cout << std::setw(3) << cnt << ": " << std::setw(3) << msg.size() << ' ' << msg << '\n';
		return 1; //std::uniform_int_distribution<int>{-1, 1}(_rnd_engine);
	}

	f8_fiber read_msg (f8_fiber&& caller, NonBlockingReadResult& result)
	{
		std::array<char, _max_msg_len> buff;
		for (;;f8_yield(caller))
		{
			std::cout << "back from yield\n";
			try
			{
				auto hlen { _hld(_rnd_engine) }, mlen { _mld(_rnd_engine) }; // variable header and body length
				result = NonBlockingReadResult::no_full_message_available;

				// read header, yield if nothing or insufficent to read
				for (int sofar{}; (sofar += read_bytes(buff.data() + sofar, hlen - sofar)) < hlen; f8_yield(caller));
				buff[hlen++] = ':'; // separator

				// read body, yield if nothing or insufficent to read
				for (int sofar{}; (sofar += read_bytes(buff.data() + hlen + sofar, mlen - sofar)) < mlen; f8_yield(caller));
				buff[hlen + mlen++] = ':'; // separator

				// read trailer, yield if nothing or insufficent to read
				for (int sofar{}; (sofar += read_bytes(buff.data() + hlen + sofar + mlen, _tlen - sofar)) < _tlen; f8_yield(caller));

				result = NonBlockingReadResult::success;
				_msg.assign(buff.data(), hlen + mlen + _tlen);
				//std::cout << "read_msg resumed\n";
			}
			catch (...)
			{
				std::cout << "read_msg exception\n";
				_eptr = std::current_exception();
			}
		}
		std::cout << "read_msg exit\n";
		return std::move(caller);
	}

public:
	Reader(std::string& msg, int rdcnt) : _msg(msg), _rdcnt(rdcnt) {}

	f8_fiber execute_reader (f8_fiber&& caller)
	{
		NonBlockingReadResult result { NonBlockingReadResult::no_full_message_available };
		f8_fiber f1(std::allocator_arg, f8_fixedsize_heap_stack(), &Reader::read_msg, this, std::placeholders::_1, std::ref(result));
		f8_fiber f2(std::allocator_arg, f8_fixedsize_heap_stack(), &Reader::read_msg, this, std::placeholders::_1, std::ref(result)); // not used

		std::cout << "f1: " << f1.get_id() << '\n';
		for (int ii{}; ii < _rdcnt && f1; ++ii)
		{
			for (bool loop{true}; loop; f8_yield(caller))
			{
				try
				{
					f8_yield(f1);
					if (_eptr)
						std::rethrow_exception(std::exchange(_eptr, nullptr));
					if (result == NonBlockingReadResult::success)
					{
						switch (process(ii + 1, _msg))
						{
						case -1:
							std::cout << "Unhandled message: " << _msg << '\n';
							++_invalid;
							break;
						case 1:
							++_processed;
							break;
						default:
							break;
						}
						loop = false;
						break;
					}
					else if (result == NonBlockingReadResult::error)
					{
						loop = false;
						++_invalid;
					}
				}
				catch (const std::exception& e)
				{
					std::cout << "read_msg: " << e.what() << '\n';
				}
			}
		}
		std::cout << "exited execute_reader loop\n";
		f8_fiber_manager::print(std::cout);
		//f1.remove(); // not needed, will be cleaned up by dtor
		std::cout << "f1: " << f1.get_id() << '\n';
		std::cout << "invalid=" << _invalid << " processed=" << _processed << '\n';
		return std::move(caller);
	}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	std::string msg;
	Reader reader(msg, argc > 1 ? std::stoi(argv[1]) : 100);
	//f8_fiber_manager::disable();
	f8_fiber f0(&Reader::execute_reader, &reader, std::placeholders::_1);
	std::cout << "f0: " << f0.get_id() << '\n';
	int yields{};

	while (f0)
	{
		f8_yield(f0);
		++yields;
	}

	std::cout << yields << " yields\nmain exit\n";
	return 0;
}

