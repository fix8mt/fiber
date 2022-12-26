#include <iostream>
#include <iomanip>
#include <array>
#include <string>
#include <string_view>
#include <random>
#include <exception>

#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
class Reader : public fiber
{
	static constexpr std::string_view b64set{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
	static const int _max_msg_len{256}, _trl_len{10};
	std::exception_ptr _eptr;
	std::mt19937 _rnd_engine{ std::random_device{}() }; // uses /dev/urandom
	std::uniform_int_distribution<int> _chardist{0, b64set.size() - 1}, _hdrdist{12, 16}, _mlendist{80, 102};

	// simulate a read that doesn't guarentee to read all that was requested
	int read_some(char *where, int cnt)
	{
		// 50% will not read all requested (will resume and try again)
		if (std::bernoulli_distribution()(_rnd_engine))
			cnt = std::uniform_int_distribution<int>{0, cnt - 1}(_rnd_engine);
		if (cnt && std::bernoulli_distribution(.002)(_rnd_engine)) // 0.2% chance of a read exception
			throw std::runtime_error("exception reading " + std::to_string(cnt) + " bytes");
		for (const auto ptr{ where + cnt }; where < ptr; ++where)
			*where = b64set[_chardist(_rnd_engine)];
		return cnt;
	}

	// override to process message
	virtual void process (std::string_view msg) const
	{
		std::cout << std::setw(3) << msg.size() << ' ' << msg << '\n';
	}

	// non-blocking re-entrant reader
	void read_nb(int rdcnt)
	{
		int mread{};
		try
		{
			for (std::array<char, _max_msg_len> buff; mread < rdcnt; ++mread)
			{
				auto hlen{ _hdrdist(_rnd_engine) }, mlen{ _mlendist(_rnd_engine) }; // variable header and body length

				// read header(var length), yield if nothing or insufficent to read
				for (int sofar{}; (sofar += read_some(buff.data() + sofar, hlen - sofar)) < hlen;)
					this_fiber::yield();
				buff[hlen++] = ' '; // separator

				// read body(var length), yield if nothing or insufficent to read
				for (int sofar{}; (sofar += read_some(buff.data() + hlen + sofar, mlen - sofar)) < mlen;)
					this_fiber::yield();
				buff[hlen + mlen++] = ' '; // separator

				// read trailer(fixed length), yield if nothing or insufficent to read
				for (int sofar{}; (sofar += read_some(buff.data() + hlen + sofar + mlen, _trl_len - sofar)) < _trl_len;)
					this_fiber::yield();

				process(std::string_view(buff.data(), hlen + mlen + _trl_len));
			}
		}
		catch (...)
		{
			_eptr = std::current_exception();
		}
		std::cout << "read_nb: " << mread << " messages read\n";
	}

public:
	Reader(int rdcnt) : fiber(&Reader::read_nb, this, rdcnt) {}
	std::exception_ptr get_exception() const { return _eptr; }
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	try
	{
		Reader reader(argc > 1 ? std::stoi(argv[1]) : 100);
		while (reader)
		{
			reader.resume();
			try
			{
				if (reader.get_exception())
					std::rethrow_exception(reader.get_exception());
			}
			catch (const std::exception& e)
			{
				std::cerr << "Reader: " << e.what() << '\n';
			}
			catch (...)
			{
				std::cerr << "Reader: unknown exception\n";
			}
		}
		std::cout << reader.get_ctxswtchs() << " yields\n";
	}
	catch (const std::exception& e)
	{
		std::cerr << "main: " << e.what() << '\n';
	}
	return 0;
}

