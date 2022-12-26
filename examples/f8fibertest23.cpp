#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <queue>
#include <random>
#include <string>
#include <fix8/f8fiber.hpp>

//-----------------------------------------------------------------------------------------
using namespace FIX8;

//-----------------------------------------------------------------------------------------
class foo
{
	std::queue<long> _queue;
	fiber _produce, _consume;

public:
   foo(int num) : _produce([this](int numtogen)
	{
		std::cout << "\tproducer:fiber entry (id:" << this_fiber::get_id() << ")\n";
		std::mt19937_64 rnde {std::random_device{}()};
		for (int cnt{}; cnt < numtogen; ++cnt)
		{
			while(_queue.size() < 5)
				_queue.push(std::uniform_int_distribution<long>(1, std::numeric_limits<long>().max())(rnde));
			std::cout << "\tproduced: " << _queue.size() << '\n';
			_consume.resume(); // switch to consumer
		}
		_consume.schedule(); // consumer is next fiber to run
		std::cout << "\tproducer:fiber exit\n";
	}, num), _consume([this]()
	{
		std::cout << "\tconsumer:fiber entry (id:" << this_fiber::get_id() << ")\n";
		while (_produce)
		{
			std::cout << "\tconsuming: " << _queue.size() << '\n';
			while(!_queue.empty())
			{
				std::cout << "\t\t" << _queue.front() << '\n';
				_queue.pop();
			}
			_produce.resume(); // switch to producer
		}
		std::cout << "\tconsumer:fiber exit\n";
	})
	{
		_produce.resume(); // switch to producer
	}
};

//-----------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
   std::cout << "main:entry\n";
   foo bar(argc > 1 ? std::stoi(argv[1]) : 10);
   std::cout << "main:exit\n";
   return 0;
}
