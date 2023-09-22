<p align="center">
  <a href="https://www.fix8mt.com"><img src="https://github.com/fix8mt/fiber/blob/main/assets/fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# fiber

### A novel C++20 fiber implementation with similar interface to `std::thread`, header-only / `x86_64` / Linux only / stackful / built-in scheduler / thread shareable

------------------------------------------------------------------------
## Introduction
This is a novel fiber implementation with a similar interface to `std::thread`. Taking any [callable](https://en.cppreference.com/w/cpp/named_req/Callable)
object, fibers execute and cooperatively yield amongst
themselves and the calling function all within a single thread. Using fibers, single threaded applications can be written as though they were multi-threaded,
with the advantage that no concurrency controls are required.
For multi-threaded applications, each thread maintains its own list of running fibers, leaving the user to implement their own concurrency controls. This
implementation allows you to move fibers between threads. This can be used to share work or scale an application by adding more fibers to new or existing threads.

Currently only `Linux/x86_64` is supported. Other platforms to be supported in the future.

| ![montest2 - example monitor application](https://github.com/fix8mt/fiber/blob/main/assets/fibermonitor1.png) |
|:--:|
| Screenshot from *`montest2`* with 20 fibers working in one thread and using `fiber_monitor`|

## Quick links
|**Link**|**Description**|
--|--
|[Wiki]( https://github.com/fix8mt/fiber/wiki)| for complete documentation|
|[API](https://github.com/fix8mt/fiber/wiki/API)| for API documentation|
|[Building](https://github.com/fix8mt/fiber/wiki/Building)| for build options and settings|
|[Monitor](https://github.com/fix8mt/fiber/wiki/Monitor)| for built-in monitor documentation|
|[Examples](https://github.com/fix8mt/fiber/wiki/Examples)| for details about all the included examples|
|[Here](https://github.com/fix8mt/fiber/blob/main/include/fix8/fiber.hpp)| for implementation|
|[Here](https://github.com/fix8mt/fiber/tree/f8_fiber_boost)| for the original `f8fiber` implementation|

## Motivation
- header-only
- `std::thread` like interface
- no external dependencies
- simplicity, lightweight
- make use of C++20 features
- `constexpr` where possible
- expand and improve interface

## Features
- **x86_64 Linux only**
- single _header-only_
- stackful fibers; stacksize configurable for each fiber
- supports any callable object (eg. function, class member, lambda expression) with optional arguments
- heap based, memory-mapped or placement stacks; user definable stack can also be used
- context switch implemented with inline assembly
- fibers can be moved to other threads (can be configured out for performance)
- `std::invoke` style ctor, with arguments perfectly forwarded to the callable
- extended API, supporting `resume`, `resume_if`, `resume_with`, `kill`, `kill_all`, `suspend`, `schedule`, `join`, `join_if`, `detach`, `resume_main`, `schedule_if`, `move`, `suspend_if`, `wait_all` and more
- built-in fiber printer
- optional built-in terminal based monitor
- helper templates including `async`, `make_fiber`, `launch_all` and `launch_all_with_params` and more
- supports `fibers` and `this_fiber` namespaces
- ctor based fiber parameter struct (POD) - including fiber name, custom stack and stack size, launch policy, launch order and auto join
- built-in round-robin scheduler
- can be used with `std::packaged_task`, `std::future`, `std::promise` and so forth
- fast, very lightweight
- fiber exceptions and general [exception handling](https://github.com/fix8mt/fiber/wiki/API#8-exception-handling)
- built-in instrumentation (can be configured out for performance)
- lots of [examples](https://github.com/fix8mt/fiber/wiki/Examples)
- full [API](https://github.com/fix8mt/fiber/wiki/API) documentation
- supports `jfiber` (similar to `std::jthread`)
- works with gcc, clang

# Examples
## 1. A simple resumable function
In the following example each iteration of `main` and `func` simply yields, printing the iteration count before and after each yield.
Note to pass a reference to the `flags` variable we need to use the `std::ref` wrapper. Before exiting `func` calls the built-in printer.
When `func` exits, control returns to `main` where testing `f0` for non-zero returns false indicating the fiber has finished.

Note that you can name a fiber using `fiber_params`. The default name for the calling thread of a fiber is `main`.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <iomanip>
#include <thread>
#include <string>
#include <fix8/fiber.hpp>
using namespace FIX8;

void func(bool& flags, int cnt)
{
   std::cout << '\t' << this_fiber::name() << " (fiber id:" << this_fiber::get_id() << ")\n";
   for (int kk{}; kk < cnt; ++kk)
   {
      std::cout << '\t' << this_fiber::name() << ':' << kk << '\n';
      this_fiber::yield();
      std::cout << '\t' << this_fiber::name() << ':' << kk << " (resumed)\n";
   }
   flags = true;
   fibers::print();
   std::cout << '\t' << this_fiber::name() << ":exit\n";
}

int main(int argc, char *argv[])
{
   std::cout << this_fiber::name() << ":entry (fiber id:" << this_fiber::get_id() << ")\n";
   bool flags{};
   fiber f0({"func"}, &func, std::ref(flags), 5);
   std::cout << "flags=" << std::boolalpha << flags << '\n';

   for (int ii{}; f0; ++ii)
   {
      std::cout << this_fiber::name() << ':' << ii << '\n';
      this_fiber::yield();
      std::cout << this_fiber::name() << ':' << ii << " (resumed)\n";
   }
   std::cout << "flags=" << std::boolalpha << flags << '\n';
   std::cout << this_fiber::name() << ":exit\n";
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest0
main:entry (fiber id:NaF)
flags=false
main:0
        func:entry (fiber id:2896)
        func:0
main:0 (resumed)
main:1
        func:0 (resumed)
        func:1
main:1 (resumed)
main:2
        func:1 (resumed)
        func:2
main:2 (resumed)
main:3
        func:2 (resumed)
        func:3
main:3 (resumed)
main:4
        func:3 (resumed)
        func:4
main:4 (resumed)
main:5
        func:4 (resumed)
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * 2896 NaF  NaF       6 0x7f9fb3632ea8 0x7f9fb3613010      352   131072 _________  99 func
1      NaF  NaF  2896      6 0x7fff7628f8a8              0        0  8388608 m________  99 main
        func:exit
main:5 (resumed)
flags=true
main:exit
$
```

</p>
</details>

## 2. A generator class
The following example implements a simple [generator](https://en.wikipedia.org/wiki/Generator_(computer_programming)) pattern.

Both the `producer()` and `consumer()` methods of `foo` are passed as callable objects to the `fiber` members `_produce` and `_consume`.

Note that the fiber ctor requires `this` for pointer to member functions. Also note the additional parameter `num` passed to `_produce`. Your callable object definition
must match the parameters passed to fiber.

After construction of the fibers, the `foo` ctor calls `_produce.resume()` which immediately switches to `producer()`.
Each iteration of the for loop generates 5 random numbers that are pushed to a queue after which `producer()` switches to `consumer()`.

Note that `consumer()` will only continue to run while the `_produce` fiber is still running, consuming all numbers pushed to the queue and when empty
switching back to `producer()`.

When `producer()` has looped `numtogen` times, it calls `_consume.schedule()` which instructs the fiber scheduler to schedule `consumer()` as the next fiber to run;
`producer()` then exits and `consumer()` is resumed (exiting a fiber causes the scheduler to switch to the next scheduled fiber) -
but since the `_produce` fiber has exited, `while (_produce)` will return `false` causing `consumer()` to also exit.

<details><summary><i>source</i></summary>
<p>

```c++
#include <queue>
#include <random>
#include <fix8/fiber.hpp>
using namespace FIX8;

class foo
{
   std::queue<long> _queue;
   fiber _produce, _consume;

   void producer(int numtogen)
   {
      std::cout << "\tproducer:entry (id:" << this_fiber::get_id() << ")\n";
      std::mt19937_64 rnde {std::random_device{}()};
      auto dist{std::uniform_int_distribution<long>(1, std::numeric_limits<long>().max())};
      for (; numtogen; --numtogen)
      {
         while(_queue.size() < 5)
            _queue.push(dist(rnde));
         std::cout << "\tproduced: " << _queue.size() << '\n';
         _consume.resume(); // switch to consumer
      }
      _consume.schedule(); // consumer is next fiber to run
      std::cout << "\tproducer:exit\n";
   }
   void consumer()
   {
      std::cout << "\tconsumer:entry (id:" << this_fiber::get_id() << ")\n";
      while (_produce)
      {
         int cnt{};
         while(!_queue.empty())
         {
            std::cout << "\t\t" << ++cnt << ": " << _queue.front() << '\n';
            _queue.pop();
         }
         std::cout << "\tconsumed: " << cnt << '\n';
         _produce.resume(); // switch to producer
      }
      std::cout << "\tconsumer:exit\n";
   }

public:
   foo(int num) : _produce(&foo::producer, this, num), _consume(&foo::consumer, this)
   {
      _produce.resume(); // switch to producer
   }
};

int main(int argc, char *argv[])
{
   std::cout << "main:entry\n";
   foo bar(argc > 1 ? std::stoi(argv[1]) : 10);
   std::cout << "main:exit\n";
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest22 5
main:entry
  producer:entry (id:4016)
  produced: 5
  consumer:entry (id:1968)
	 1: 1349602525532272804
	 2: 316583067409009491
	 3: 1020347932332564514
	 4: 3829997051190076899
	 5: 947478241006829049
  consumed: 5
  produced: 5
	 1: 3617976082375098752
	 2: 3972849389773859934
	 3: 7998668485491537141
	 4: 4650831140486621276
	 5: 7332043501653828395
  consumed: 5
  produced: 5
	 1: 2230847709081134901
	 2: 6965146163979072417
	 3: 6029855094014219769
	 4: 1063218563532550271
	 5: 1444553618767029414
  consumed: 5
  produced: 5
	 1: 4198632358838426114
	 2: 9056029556599489020
	 3: 8320555710292082575
	 4: 2721317769035964708
	 5: 4913464453217416140
  consumed: 5
  produced: 5
	 1: 3735451366216011356
	 2: 4377846252953671965
	 3: 5218125716664024839
	 4: 6370858901001428693
	 5: 6699210929525116824
  consumed: 5
  producer:exit
  consumer:exit
main:exit
$
```

</p>
</details>

## 3. Resumable non-blocking message reader
This example demonstrates the use of a fiber to incrementally read a message, yielding to the caller when there is insuffient data
to be read. The caller can perform other processing, and return to the reader to resume reading more of a message. When a message is fully
read, a process function is called.

The fiber entry point `read_nb(int rdcnt)` will read `rdcnt` messages. Each message consists of a variable header (length 12 - 16 bytes);
a variable body (length 80 - 102) and a fixed length trailer (length 10). Bytes are chosen randomly from a Base64 string.
Each component of the message is read using the method `read_some()`
which will randomly choose to _not_ fully complete a read (50% of the time), randomly reading some count less than requested. Reading will continue, yielding to
the caller when insufficient data is available.

`read_some()` will also randomly throw a `std::runtime_error`. `read_nb()` catches any exception and assigns it to a `std::exception_ptr`. If
an exception is caught, the method will not attempt to read any more messages, exiting the fiber and returning to `main`. When control is
returned, `main` checks for a non-empty `std::exception_ptr` and rethrows the stored exception. `main` will continue to yield to the reader
(`resume()`) while the reader fiber is still running (`while (reader)`).

Note our reader class inherits from `fiber`, and constructs the fiber with a pointer to a member class, a `this` and an integer
parameter.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <iomanip>
#include <array>
#include <string>
#include <string_view>
#include <random>
#include <exception>
#include <fix8/fiber.hpp>
using namespace FIX8;

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

   // non-blocking resumable reader
   void read_nb(int rdcnt)
   {
      int mread{};
      try
      {
         for (std::array<char, _max_msg_len> buff; mread < rdcnt; ++mread)
         {
            auto hlen{ _hdrdist(_rnd_engine) }, mlen{ _mlendist(_rnd_engine) }; // variable header and body length

            // read header(var length), yield if nothing or insufficient to read
            for (int sofar{}; (sofar += read_some(buff.data() + sofar, hlen - sofar)) < hlen;)
               this_fiber::yield();
            buff[hlen++] = ' '; // separator

            // read body(var length), yield if nothing or insufficient to read
            for (int sofar{}; (sofar += read_some(buff.data() + hlen + sofar, mlen - sofar)) < mlen;)
               this_fiber::yield();
            buff[hlen + mlen++] = ' '; // separator

            // read trailer(fixed length), yield if nothing or insufficient to read
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
```

</p>
</details>

<details><summary><i>output</i></summary>
In this example, 20 messages were requested however an exception was thrown reading the 13th message, and reading stopped. Note also the number
of yields back to the caller are reported.
<p>

```bash
$ ./fibertest26 20
124 LctFjN/09NBZ mDPtFqvztmHWflL67CqtFWiJmG6FKsYgEnMzDRPCf0xm5FysuVHh+x0NrSISh4Z9p4PptyxP+w1ewI4rsBAAG8Hb76nbKbt8yLNg NhDTxR2Asw
113 1mx5RNdVJisFj lEc8IFOQ1uvlV2e8w5WOjT4+I3atpf6cor4pW6qAhggUQ6nIoGbM+BVTZ8bsjoLVNmIVoQIH/G1P5etK0gEg7cIa 2YI/4unCoF
114 mCQ7o0gV+oC6 ljEQArWk+9Z5Kk0tZasyowLb7rBAJH7Ien/MqXT6hqI7ycrSCVtf9ObUotjTgLpWpF77dGD3u478gPTwHEIKbw8Bg8 uIGaY/BghC
106 DuIReo+2HZqC XvCz4YEh+gsKD9hKlaMy7PWxB8LThVQGtOVEyJ7TBp5HwKEYJkOwauOU0YjPmUjEnKFDxwubv2RVtM8u/0 3xEcEbzrXl
109 fTmQIoTJ2Cd0E+p LwwawaUbyvHG+3hqFDHL41pDTrM1tA3VRO2LCK/ppCCdBb8DQqVUiVI3iXL9zZ489jjtRNmP7LiuHbhxdo 7fp38/3mQX
130 GuTL6VWln+voJ+w5 Xkk4NRoWvLHUo40qBF09I6sbN0PJ/YRXBWab0iiwhGrl9ylgq/3jcm17j5iJIvzVzdvup+IOMh+ScVHQ4EeibkSgI9UwI0r9tHJh6s IaNuK3ZDLh
123 JEtJ8Q8ypfHshtZ0 OBz39mjNSfy0Gvnf1cmhT0iSY5pGzG+ovr24QVFE2CDNgjCV8+qzzGCrcMMhSoToL6UqTC0T/SHO2zt8DtvVARq6HrjZsGl CdDavW90LN
127 jgHj1nd9QZe5HH Q618VjDY8dz7VNOCuGrHELFoJ2UIFbKBf+otemUv6mxAksA06sdd6Y3CG7SeuxGrHnjNo4xI+/6O73AEiYpI0xl+8Lrtk4w6B8OP8 v5y9/oAcKz
126 OLDdCx1fnURzM/fl HcdeeLbOc5bG9oTbogTdn2eiduU/NuDoojEn2/RrF0bXPl+sernRHXHbukASoBFYdpxVm1CMYRC4KoeKEueQMNeFz9rbcuDjxl 7Nhg9oaAh5
111 0DK3NYzIokmtHMC tF7Ld99kdIPJYNe/WeCEGqTD20kvJPmeceyVDHbAASaHaQtrFzX8zPcXekBHxujUpi2J50Xr6l0Herd9uQA+ hQs0pwOY8J
116 PnAEXeLfo68k/ SMBLhvBh5ZqCs+WgLhPEPpzvmt42bsWy/E7HkYqG2BOUe2LgVNdq1aAYVtcfAZ4SJlalNpbA1Lzv0olgweXtqlLx+Z1 rvMDIX+24O
128 4RvyYIO+VDSalI xMjWP3bKvFAxedgdCaLT605zPW4/eFv2iOhrVrVtnMZSZM4OZjEtg/sBj1iORVtoeLmVBnSSTuoWOIJn7K5avDf6kI3tKxj1N9ESFA EhX45Ucg9X
read_nb: 12 messages read
Reader: exception reading 5 bytes
28 yields
$
```

</p>
</details>

## 4. Detached fiber workpiece
In this example, four detached fibers are created.

When main ends, the detached fibers are activated in creation order. Each fiber takes a lambda expression as its callable object,
which prints from an array constructed with every fourth word until the array is exhausted.

Note the ctor for each fiber takes a reference to one of the `string_view` arrays, passed as an argument.
See fibertest10.cpp

<details><summary><i>source</i></summary>
<p>

```c++
#include <array>
#include <iostream>
#include <fix8/fiber.hpp>
using namespace FIX8;

int main()
{
   static constexpr const std::array wordset
   {
      std::array { R"("I)",      "all",   "said",  "It’s",     "I’m",         "\n –",        },
      std::array { "am",         "of",    "no",    "because",  "doing",       "Albert",      },
      std::array { "thankful",   "those", "to",    "of",       "it",          "Einstein\n"   },
      std::array { "for",        "who",   "me.",   "them",     R"(myself.")", ""             },
   };

   for (const auto& pp : wordset)
   {
      fiber ([](const auto& words)
      {
         for (auto qq : words)
         {
            std::cout << qq << ' ';
            this_fiber::yield();
         }
      }, pp).detach();
   }

   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest10
"I am thankful for all of those who said no to me. It’s because of them I’m doing it myself."
 – Albert Einstein
$
```

</p>
</details>

## 5. Another detached fiber workpiece using `launch_all` and `launch_all_with_params`
Similar to the previous example, four detached fibers are created in a new thread. Two versions of this are shown.
```c++
template<std::invocable... Fns>
constexpr void launch_all(Fns&& ...funcs);

template<typename Ps, std::invocable Fn, typename... Fns>
constexpr void launch_all_with_params(Ps&& params, Fn&& func, Fns&& ...funcs);
```
To demonstrate the use of `launch_all` and `launch_all_with_params`,
one example simply creates the fibers in the order they are defined; the second example creates the fibers with a specified launch order (hence the need for
'with params').

The `launch_all` and `launch_all_with_params` templates always detach the fibers they create. Two additional versions are also provided - `launch_all_n` and `launch_all_with_params_n`
which create fibers to a supplied container.  See `fibertest27.cpp` for an example.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <array>
#include <utility>
#include <fix8/fiber.hpp>
using namespace FIX8;

int main()
{
   static constexpr const std::array wordsets
   {
      std::array { R"("I )",     "all ",     "said ",    "It’s ",    "I’m ",        ""             },
      std::array { "for ",       "who ",     "me. ",     "them ",    "myself.\"\n", ""             },
      std::array { "am ",        "of ",      "no ",      "because ", "doing ",      " - Albert ",  },
      std::array { "thankful ",  "those ",   "to ",      "of ",      "it ",         "Einstein\n"   },
   };

   static const auto func([](const auto& words)
   {
      for (auto pp : words)
      {
         std::cout << pp;
         this_fiber::yield();
      }
   });

   std::thread([]()
   {
      launch_all // will print in fiber work order
      (
         std::bind(func, wordsets[0]),
         std::bind(func, wordsets[1]),
         std::bind(func, wordsets[2]),
         std::bind(func, wordsets[3])
      );
   }).join();

   launch_all_with_params // will print in specified order
   (
      fiber_params{.launch_order=0}, std::bind(func, wordsets[0]),
      fiber_params{.launch_order=3}, std::bind(func, wordsets[1]),
      fiber_params{.launch_order=1}, std::bind(func, wordsets[2]),
      fiber_params{.launch_order=2}, std::bind(func, wordsets[3])
   );

   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest12
"I for am thankful all who of those said me. no to It’s them because of I’m myself."
doing it  - Albert Einstein
"I am thankful for all of those who said no to me. It’s because of them I’m doing it myself."
 - Albert Einstein
$
```

</p>
</details>

## 6. Example with `std::packaged_task` and `std::future`
Using `std::packaged_task` we create a task - a lambda expression taking an int and returning an int. We then obtain a `std::future` from the task, and `std::move`
the task to a `fiber`. The fiber will call the task execute operator.

The fiber and main switch control between each other until the fiber has completed. When it completes, it returns a value which the task makes available to the future.
This value is then obtained by the call to `myfuture.get()`.

<details><summary><i>source</i></summary>
<p>

```c++
#include <functional>
#include <future>
#include <fix8/fiber.hpp>
using namespace FIX8;

int main(void)
{
   std::packaged_task task([](int arg)
   {
      std::cout << "\tstarting sub\n";
      for (int ii{}; ii < arg; this_fiber::yield())
         std::cout << "\tsub: " << ++ii << '\n';
      std::cout << "\tleaving sub\n";
      return arg * 100;
   });
   auto myfuture { task.get_future() };
   fiber myfiber(std::move(task), 10);
   for (int ii{}; myfiber; this_fiber::yield())
      std::cout << "main: " << ++ii << '\n';
   std::cout << "Future result = " << myfuture.get() << "\nExiting from main\n";
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest3
main: 1
        starting sub
        sub: 1
main: 2
        sub: 2
main: 3
        sub: 3
main: 4
        sub: 4
main: 5
        sub: 5
main: 6
        sub: 6
main: 7
        sub: 7
main: 8
        sub: 8
main: 9
        sub: 9
main: 10
        sub: 10
main: 11
        leaving sub
Future result = 1000
Exiting from main
$
```

</p>
</details>

## 7. Resuming with a different callable object
This example is similar to example #1. However, after a few iterations, the main function resumes the fiber with a new function (in fact it is the
same function with different arguments). The original function stack is completely overwritten by the new resumed function. To demonstrate the impact of this on
the stack, a simple object is created that takes a single integer parameter. The ctor shows the parameter value and its address. Note that when the
new resumed function is called, the value is different but the address is the same. Note the dtor for `foo(10)` is never called. It is important
to keep this in mind when using `resume_with`.

Also note that in order to rename the fiber, we must use `set_params()`. This can be conveniently bundled with the `resume_with()` call since it returns a `fiber&`.
The usual test for a running fiber is used to signal the exiting of the application.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <string>
#include <fix8/fiber.hpp>
using namespace FIX8;

struct foo
{
   int _a;
   foo(int a) : _a(a) { std::cout << "foo(" << _a << ",&_a=" << &_a << ")\n"; }
   ~foo() { std::cout << "~foo(" << _a << ")\n"; }
};

void sub(int arg, int spacer)
{
   foo a(arg);
   const auto tabs { std::string(spacer, '\t') };
   std::cout << tabs << "starting " << this_fiber::name() << '\n';
   for (int ii{}; ii < arg; )
   {
      std::cout << tabs << this_fiber::name() << ": " << ++ii << '\n';
      this_fiber::yield();
   }
   std::cout << tabs << "leaving " << this_fiber::name() << '\n';
}

int main()
{
   int ii{};
   for (fiber myfiber({"sub"}, &sub, 10, 1); myfiber; this_fiber::yield())
   {
      std::cout << "main: " << ++ii << '\n';
      if (ii == 9)
      {
         myfiber.set_params("sub1").resume_with(&sub, 5, 2);
         for (int jj{}; myfiber; this_fiber::yield())
            std::cout << "main1: " << ++jj << '\n';
      }
   }
   std::cout << "Exiting from main\n";
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest8
main: 1
foo(10,&_a=0x7f91323b9f8c)
        starting sub
        sub: 1
main: 2
        sub: 2
main: 3
        sub: 3
main: 4
        sub: 4
main: 5
        sub: 5
main: 6
        sub: 6
main: 7
        sub: 7
main: 8
        sub: 8
main: 9
foo(5,&_a=0x7f91323b9f8c)
                starting sub1
                sub1: 1
main1: 1
                sub1: 2
main1: 2
                sub1: 3
main1: 3
                sub1: 4
main1: 4
                sub1: 5
main1: 5
                leaving sub1
~foo(5)
Exiting from main
$
```

</p>
</details>

## 8. Using `jfiber`
A `jfiber` differs from a `fiber` in that it automatically joins on destruction, similar to C++20 `std::jthread`. This example demonstrates the difference between the two.
With no command line arguments, the application will create a `jfiber` otherwise a normal `fiber` is created. With the normal fiber, `main` will exit before
the fiber has completed. The default behaviour is the same as a `std::thread` that has not been joined - `terminate`. But with the `jfiber`, when `main` exits, the jfiber
will join, so that the fiber is resumed. When the fiber has finished, the application can end.

Note the use of the helper `make_fiber` and `fiber_ptr`.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <functional>
#include <fix8/fiber.hpp>
using namespace FIX8;
using namespace std::literals;

void doit()
{
   std::cout << "\tstarting " << this_fiber::name() << '\n';
   this_fiber::yield();
   for(int ii{}; ii < 10; std::this_thread::sleep_for(100ms))
      std::cout << '\t' << this_fiber::name() << ": " << ++ii << '\n';
   std::cout << "\tleaving " << this_fiber::name() << '\n';
}

int main(int argc, char *argv[])
{
   fiber_ptr fb { argc > 1 ? make_fiber({"fiber"}, &doit) : make_fiber<jfiber>({"jfiber"}, &doit) };
   this_fiber::yield();
   fibers::print();
   std::cout << "Exiting from main\n";
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest25 1
        starting fiber
#      fid  pfid prev   ctxs      stack ptr    stack alloc     depth  stacksz     flags ord name
0    * NaF  NaF  2544      2 0x7ffffe9423a8              0        0  8388608 m________  99 main
1      2544 NaF  NaF       1 0x7fa75c873e58 0x7fa75c854010      432   131072 _________  99 fiber
Exiting from main
fiber has exited. Terminating application.
terminate called without an active exception
Aborted (core dumped)
$
$ ./fibertest25
        starting jfiber
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  5168      2 0x7ffe8e345e68              0        0  8388608 m________  99 main
1      5168 NaF  NaF       1 0x7fe177935e58 0x7fe177916010      432   131072 ______j__  99 jfiber
Exiting from main
        jfiber: 1
        jfiber: 2
        jfiber: 3
        jfiber: 4
        jfiber: 5
        jfiber: 6
        jfiber: 7
        jfiber: 8
        jfiber: 9
        jfiber: 10
        leaving jfiber
$
```

</p>
</details>

## 9. Moving a fiber to another thread
In this example we are creating two fibers in the main thread and another fiber in a different thread. All fibers are executing the same function which loops 10 times, yielding on each iteration.
Each fiber prints the iteration count, as does the main fiber in both threads.
After five loops, the main thread moves one of its fibers to the other thread (`t1`). At various intervals, both threads `fibers::print()` showing which thread has which fibers:
- The first print shows the second thread's `main` and `first` fibers.
- The second print shows the main thread's `main`, `first` and `second` fibers.
- The third print shows the remaining main thread's fibers `main` and `second` (after `first` was moved to the other thread).
- The fourth print shows the remaining main thread's fibers `main` (after `second` has finished).
- The fifth print shows the second thread's fibers `main`, `thread:first` and `first` (after `first` has been moved to this thread).

When the main thread finishes, it does a thread join, and waits till the second thread has finished.

<details><summary><i>source</i></summary>
<p>

```c++
#include <thread>
#include <fix8/fiber.hpp>
using namespace FIX8;
using namespace std::chrono_literals;

void sub(int arg)
{
   std::cout << "\tstarting " << this_fiber::name() << '\n';
   for (int ii{}; ii < arg; this_fiber::yield())
   {
      std::cout << '\t' << std::this_thread::get_id() << ' ' << this_fiber::name() << ' ' << ++ii << '\n';
      std::this_thread::sleep_for(250ms);
   }
   std::cout << "\tleaving " << this_fiber::name() << '\n';
}

int main(void)
{
   fiber f0({"first"}, &sub, 10), f2({"second"}, &sub, 10);
   std::thread t1([]()
   {
      jfiber ft1({"thread:first"}, &sub, 10);
      for (int ii{}; fibers::has_fibers(); ++ii)
      {
         std::cout << "main1 " << ii << '\n';
         switch(ii)
         {
         case 1:
         case 9:
            fibers::print();
            [[fallthrough]];
         default:
            std::this_thread::sleep_for(250ms);
            this_fiber::yield();
            break;
         }
      }
      std::cout << std::this_thread::get_id() << " Exiting from main1\n";
   });

   std::this_thread::sleep_for(100ms);

   for (int ii{}; fibers::has_fibers(); ++ii)
   {
      std::cout << "main " << ii << '\n';
      switch(ii)
      {
      case 5:
         std::cout << "transferring " << f0.get_id() << " from "
            << std::this_thread::get_id() << " to " << t1.get_id() << '\n';
         f0.move(t1.get_id());
         break;
      case 4:
      case 9:
         fibers::print();
         [[fallthrough]];
      default:
         this_fiber::yield();
         break;
      }
   }
   std::cout << "waiting at join...\n";
   fibers::print();
   t1.join();
   std::cout << std::this_thread::get_id() << " Exiting from main\n";
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest24
main1 0
main 0
        starting first
        140162105964928 first 1
        starting thread:first
        140162099050176 thread:first 1
        starting second
        140162105964928 second 1
main1 1
Thread id 140162099050176
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  640       2 0x7f7a081fea18              0        0  8388608 m________  99 main
1      640  NaF  NaF       1 0x7f7a000209e8 0x7f7a00000ba0      432   131072 ______j__  99 thread:first
main 1
        140162105964928 first 2
        140162099050176 thread:first 2
        140162105964928 second 2
main1 2
main 2
        140162105964928 first 3
        140162099050176 thread:first 3
        140162105964928 second 3
main1 3
main 3
        140162105964928 first 4
        140162099050176 thread:first 4
        140162105964928 second 4
main1 4
main 4
Thread id 140162105964928
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  9152      5 0x7ffc66f3bb38              0        0  8388608 m________  99 main
1      2736 NaF  NaF       4 0x7f7a088e2e58 0x7f7a088c3010      432   131072 _________  99 first
2      9152 NaF  2736      4 0x7f7a08893e58 0x7f7a08874010      432   131072 _________  99 second
        140162105964928 first 5
        140162099050176 thread:first 5
        140162105964928 second 5
main1 5
main 5
transferring 2736 from 140162105964928 to 140162099050176
        140162105964928 second 6
        140162099050176 thread:first 6
main 6
        140162105964928 second 7
        140162099050176 first 6
main 7
        140162105964928 second 8
main1 6
main 8
        140162105964928 second 9
        140162099050176 thread:first 7
main 9
Thread id 140162105964928
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  9152     10 0x7ffc66f3bb38              0        0  8388608 m________  99 main
1      9152 NaF  NaF       9 0x7f7a08893e58 0x7f7a08874010      432   131072 _________  99 second
        140162105964928 second 10
        140162099050176 first 7
main 10
        leaving second
main 11
waiting at join...
Thread id 140162105964928
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  9152     12 0x7ffc66f3bb38              0        0  8388608 m________  99 main
main1 7
        140162099050176 thread:first 8
        140162099050176 first 8
main1 8
        140162099050176 thread:first 9
        140162099050176 first 9
main1 9
Thread id 140162099050176
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  2736     10 0x7f7a081fea18              0        0  8388608 m________  99 main
1      640  NaF  NaF       9 0x7f7a000209e8 0x7f7a00000ba0      432   131072 ______j__  99 thread:first
2      2736 NaF  640       9 0x7f7a088e2e58 0x7f7a088c3010      432   131072 ________v  99 first
        140162099050176 thread:first 10
        140162099050176 first 10
main1 10
        leaving thread:first
        leaving first
main1 11
140162099050176 Exiting from main1
140162105964928 Exiting from main
$
```

</p>
</details>

## 10. Creating fibers using different callable objects
This example creates 6 fibers:
| **Var** | **Type** | **Description** | **Parameters** |
--|--|--|--
| `sub_co` | function | Creates fiber with stacksz=2048 calling function `void doit(int arg)` | `3` |
| `sub_co1` | member function | Creates fiber with stacksz=16384 calling `void foo::sub2()` | `sub2` calls `doit(4)` |
| `sub_co2` | member function | Creates fiber with stacksz=32768 calling `void foo::sub(int arg)` | `5` |
| `sub_co3` | member function | Creates fiber with stacksz=8192 calling `void foo::sub1(int arg, const char *str)` | `8.0`, "hello"|
| `sub_co4` | member function | Creates fiber calling `void foo::sub3(int arg, const char *str)` | `12`, "there"|
| `sub_lam` | lambda | Creates fiber named "sub lambda" with mapped stack with default stacksz calling supplied lambda | `15` |

Once the fibers have been created, the main loop begins. Each iteration simply yields, allowing one of the fibers to execute. The `has_fibers` function
returns true if any runnable fibers are available. Note that the first iteration performs some specific action - calling print then switching to `sub_co3`. When control
finally returns to main another print is performed. Examining the two print outputs reveals the changes that have occurred in the fiber manager. Also
note that since `sub_co3` is the first fiber to actually execute, you can see the string `hello` printed followed by `sub8` (which is the number of
iterations that `sub_co3` will perform).

Notice each fiber uses `this_fiber::name` to name itself with exception of `sub_lam` which uses `fiber_params` to name itself.

Each of the fibers prints a start message and loops for a different number of times, printing each iteration each time and
finally printing a leaving message before finishing. Main similarly prints each iteration.

In fiber `sub_co4`, the fiber is suspended for 100ms each loop. This means that other fibers will get a larger share of the available cpu.
Eventually only this fiber is left running as can be seen by the `sub12` messages.

The sizes of various objects are printed at the end.

<details><summary><i>source</i></summary>
<p>

```c++
#include <iostream>
#include <functional>
#include <fix8/fiber.hpp>
using namespace FIX8;
using namespace std::literals;

void doit(int arg)
{
   std::cout << this_fiber::name("sub"s + std::to_string(arg));
   std::cout << "\tstarting " << arg << '\n';
   for (int ii{}; ii < arg; )
   {
      std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
      this_fiber::yield();
   }
   std::cout << "\tleaving " << arg << '\n';
   fibers::print();
}

struct foo
{
   void sub(int arg)
   {
      doit(arg);
   }
   void sub1(int arg, const char *str)
   {
      std::cout << str << '\n';
      doit(arg);
   }
   void sub3(int arg, const char *str)
   {
      auto st { "sub"s + std::to_string(arg) };
      this_fiber::name(st);
      std::cout << "\tsub2 starting " << arg << '\n';
      for (int ii{}; ii < arg; )
      {
         std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
         //this_fiber::sleep_until(std::chrono::steady_clock::now() + 500ms);
         this_fiber::sleep_for(100ms);
      }
      std::cout << "\tsub2 leaving " << arg << '\n';
   }
   void sub2()
   {
      doit(4);
   }
};
int main(void)
{
   foo bar;
   fiber sub_co({.stacksz=2048}, &doit, 3),
         sub_co1({.stacksz=16384}, &foo::sub2, &bar),
         sub_co2({.stacksz=32768}, &foo::sub, &bar, 5),
         sub_co3({.stacksz=8192}, &foo::sub1, &bar, 8., "hello"),
         sub_co4(std::bind(&foo::sub3, &bar, 12, "there"));
   fiber sub_lam({.name="sub lambda",.stack=std::make_unique<f8_fixedsize_mapped_stack>()}, [](int arg)
   //char stack[4096];
   //fiber sub_lam({.name="sub lambda",.stacksz=sizeof(stack),.stack=std::make_unique<f8_fixedsize_placement_stack>(stack)}, [](int arg)
   {
      std::cout << "\tlambda starting " << arg << '\n';
      for (int ii{}; ii < arg; )
      {
         std::cout << '\t' << this_fiber::name() << ' ' << arg << ": " << ++ii << '\n';
         this_fiber::yield();
      }
      std::cout << "\tlambda leaving " << arg << '\n';
   }, 15);
   for (int ii{}; fibers::has_fibers(); ++ii)
   {
      if (ii == 0)
      {
         fibers::print(std::cout);
         sub_co3.resume();
         fibers::print(std::cout);
      }
      this_fiber::yield();
      std::cout << "main: " << std::dec << ii << '\n';
   }
   std::cout << "Exiting from main\nSizes\n";
   std::cout << "fiber: " << sizeof(fiber) << '\n';
   std::cout << "fiber::cvars: " << sizeof(fiber::cvars) << '\n';
   std::cout << "fiber::all_cvars: " << sizeof(fiber::all_cvars) << '\n';
   std::cout << "fiber_id: " << sizeof(fiber_id) << '\n';
   std::cout << "fiber_base: " << sizeof(fiber_base) << '\n';
   std::cout <<"fiber_params: " << sizeof(fiber_params) << '\n';
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```
$ ./fibertest
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  NaF       1              0              0        0  8388608 m________  99 main
1      9072 NaF  NaF       0 0x563f8b3a9690 0x563f8b3a8ee0       72     2048 _____n___  99
2      2352 NaF  NaF       0 0x563f8b3adb60 0x563f8b3a9bb0       72    16384 _____n___  99
3      8864 NaF  NaF       0 0x563f8b3b5be0 0x563f8b3adc30       72    32768 _____n___  99
4      1728 NaF  NaF       0 0x563f8b3b7c40 0x563f8b3b5c90       72     8192 _____n___  99
5      2976 NaF  NaF       0 0x7f905fc76fc0 0x7f905fc57010       72   131072 _____n___  99
6      1728 NaF  NaF       0 0x7f905f913fb0 0x7f905f8f4000       72   131072 _____n___  99 sub lambda
hello
sub8    starting 8
        sub8 8: 1
sub4    starting 4
        sub4 4: 1
sub5    starting 5
        sub5 5: 1
sub3    starting 3
        sub3 3: 1
        sub2 starting 12
        sub12 12: 1
        lambda starting 15
        sub lambda 15: 1
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * NaF  NaF  1728      2 0x7ffebd5d1368              0        0  8388608 m________  99 main
1      1728 NaF  NaF       1 0x563f8b3b7a68 0x563f8b3b5c90      544     8192 _________  99 sub8
2      2352 NaF  1728      1 0x563f8b3ad998 0x563f8b3a9bb0      528    16384 _________  99 sub4
3      8864 NaF  2352      1 0x563f8b3b5a08 0x563f8b3adc30      544    32768 _________  99 sub5
4      9072 NaF  8864      1 0x563f8b3a94b8 0x563f8b3a8ee0      544     2048 _________  99 sub3
5      2976 NaF  9072      1 0x7f905fc76dd8 0x7f905fc57010      560   131072 __s______  99 sub12
6      1728 NaF  2976      1 0x7f905f913e68 0x7f905f8f4000      400   131072 _________  99 sub lambda
        sub8 8: 2
        sub4 4: 2
        sub5 5: 2
        sub3 3: 2
        sub lambda 15: 2
main: 0
        sub8 8: 3
        sub4 4: 3
        sub5 5: 3
        sub3 3: 3
        sub lambda 15: 3
main: 1
        sub8 8: 4
        sub4 4: 4
        sub5 5: 4
        leaving 3
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * 9072 NaF  8864      4 0x563f8b3a94b8 0x563f8b3a8ee0      544     2048 _________  99 sub3
1      1728 NaF  9072      3 0x7f905f913e68 0x7f905f8f4000      400   131072 _________  99 sub lambda
2      NaF  NaF  1728      4 0x7ffebd5d1368              0        0  8388608 m________  99 main
3      1728 NaF  NaF       4 0x563f8b3b7a68 0x563f8b3b5c90      544     8192 _________  99 sub8
4      2976 NaF  9072      1 0x7f905fc76dd8 0x7f905fc57010      560   131072 __s______  99 sub12
5      2352 NaF  1728      4 0x563f8b3ad998 0x563f8b3a9bb0      528    16384 _________  99 sub4
6      8864 NaF  2352      4 0x563f8b3b5a08 0x563f8b3adc30      544    32768 _________  99 sub5
        sub lambda 15: 4
main: 2
        sub8 8: 5
        leaving 4
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * 2352 NaF  1728      5 0x563f8b3ad998 0x563f8b3a9bb0      528    16384 _________  99 sub4
1      8864 NaF  2352      4 0x563f8b3b5a08 0x563f8b3adc30      544    32768 _________  99 sub5
2      9072 NaF  8864      4 0x563f8b3a9588 0x563f8b3a8ee0      336     2048 _f_______  99 sub3
3      1728 NaF  9072      4 0x7f905f913e68 0x7f905f8f4000      400   131072 _________  99 sub lambda
4      NaF  NaF  1728      5 0x7ffebd5d1368              0        0  8388608 m________  99 main
5      2976 NaF  9072      1 0x7f905fc76dd8 0x7f905fc57010      560   131072 __s______  99 sub12
6      1728 NaF  NaF       5 0x563f8b3b7a68 0x563f8b3b5c90      544     8192 _________  99 sub8
        sub5 5: 5
        sub lambda 15: 5
main: 3
        sub8 8: 6
        leaving 5
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * 8864 NaF  1728      6 0x563f8b3b5a08 0x563f8b3adc30      544    32768 _________  99 sub5
1      1728 NaF  8864      5 0x7f905f913e68 0x7f905f8f4000      400   131072 _________  99 sub lambda
2      2976 NaF  9072      1 0x7f905fc76dd8 0x7f905fc57010      560   131072 __s______  99 sub12
3      NaF  NaF  1728      6 0x7ffebd5d1368              0        0  8388608 m________  99 main
4      1728 NaF  NaF       6 0x563f8b3b7a68 0x563f8b3b5c90      544     8192 _________  99 sub8
        sub lambda 15: 6
main: 4
        sub8 8: 7
        sub lambda 15: 7
main: 5
        sub8 8: 8
        sub lambda 15: 8
main: 6
        leaving 8
#      fid  pfid prev   ctxs      stack ptr    stack alloc    depth  stacksz     flags ord name
0    * 1728 NaF  NaF       9 0x563f8b3b7a68 0x563f8b3b5c90      544     8192 _________  99 sub8
1      2976 NaF  9072      1 0x7f905fc76dd8 0x7f905fc57010      560   131072 __s______  99 sub12
2      1728 NaF  1728      8 0x7f905f913e68 0x7f905f8f4000      400   131072 _________  99 sub lambda
3      NaF  NaF  1728      9 0x7ffebd5d1368              0        0  8388608 m________  99 main
        sub lambda 15: 9
main: 7
        sub lambda 15: 10
main: 8
        sub lambda 15: 11
main: 9
        sub lambda 15: 12
main: 10
        sub lambda 15: 13
main: 11
        sub lambda 15: 14
main: 12
        sub lambda 15: 15
main: 13
        lambda leaving 15
main: 14
        sub12 12: 2
main: 15
        sub12 12: 3
main: 16
        sub12 12: 4
main: 17
        sub12 12: 5
main: 18
        sub12 12: 6
main: 19
        sub12 12: 7
main: 20
        sub12 12: 8
main: 21
        sub12 12: 9
main: 22
        sub12 12: 10
main: 23
        sub12 12: 11
main: 24
        sub12 12: 12
main: 25
        sub2 leaving 12
main: 26
main: 27
Exiting from main
Sizes
fiber: 16
fiber::cvars: 248
fiber::all_cvars: 120
fiber_id: 8
fiber_base: 224
fiber_params: 48
$
```

</p>
</details>

## 11. Example from Dilawar's blog "An Example of Boost Fiber"
The following example is based on [Dilawar's Blog](https://dilawar.github.io/posts/2021/2021-11-14-example-boost-fiber/). The following description is also based on the text in the blog.

>The program has two lambdas: `print_a` prints a and `print_b` prints b and then launches a thread that prints B (in detached mode).
We created a shared variable ii initialized to 0. We use this a global state. We create two detached fibers.
First one keeps calling `print_a` till `ii < 20`. Similarly, the second one loops on `print_b` till `ii < 20`. Both increment ii by 1. When `ii = 20`, both fibers should be able to join.
Let’s guess the output of this program. It is most likely to be the same as if std::threads were used instead of fiber.
X is printed first? Yes. Note that `detach()` is called on each fibers so neither of their functions are called. They are put in the background.

>Control passes to the fiber manager at return 0; when it asks the fibers to join.
In fact, you can put more computations after the `std::cout << 'X';` statement and it would be computed before any fiber is called.
As soon as we try to return from the main, fiber manager is asked to join the fibers. The first fiber awakes, a is printed and the fiber yields the control to the manager.
Fiber manager then wakes up the second fiber (who was waiting in the queue) that prints b and also launched a thread in the background that prints B.
We can not be sure if B will be printed immediately after the b (it is a std::thread). `print_b` yields the control and goes to sleep.
The fiber manager wakes up first fiber again that calls `print_a` again and a is printed and so on. Note that ii is also incremented every time either of the fibers are called.
When ii hits 20, both fibers terminates and joined and the main function return 0;.

>So we have `print_a` called 10 times and `print_b` is also called 10 times. In the output, we should have 10 a's, 10 b's and 10 B's. B may not strictly follow b but b must come after the a.
Note that the location of B is not deterministic.

<details><summary><i>source</i></summary>
<p>

```c++
#include <cstdio>
#include <thread>
#include <fix8/fiber.hpp>
using namespace FIX8;

int main()
{
   static int ii{};

   fiber([]() // print_a
   {
      do
      {
         std::printf("%c", 'a');
         this_fiber::yield();
      }
      while (++ii < 20);
   }).detach();

   fiber([]() // print_b
   {
      do
      {
         std::printf("%c", 'b');
         std::thread([]() { std::printf("%c", 'B'); }).detach();
         this_fiber::yield();
      }
      while (++ii < 20);
   }).detach();

   std::printf("%c", 'X');
   return 0;
}
```

</p>
</details>

<details><summary><i>output</i></summary>
<p>

```bash
$ ./fibertest7
XabababBabBBabBababBabBabBBabBaB
$
$ ./fibertest7
XabababBabBBabBabBabBababBabBaBB
$
$ ./fibertest7
XabababBabBBabBabBababBababBBBaB
$
```

</p>
</details>

