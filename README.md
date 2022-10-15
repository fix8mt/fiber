<p align="center">
  <a href="https://www.fix8mt.com"><img src="assets/fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# f8fiber
### A novel C++17 fiber implementation with some ideas from `boost::fiber`, header-only / x86_64 / Linux only / stackful / built-in scheduler

------------------------------------------------------------------------
## Introduction
This is a novel fiber implementation, with some ideas from boost::fiber.
- **x86_64 Linux only**
- single _header-only_
- auto cleanup - fiber will be removed when it goes out of scope without needing it to 'return'
- context switch implemented with inline assembly
- `std::bind` can be omitted with args forwarded by ctor
- exception safe - all exceptions can be captured by a `std::exception_ptr` within the fiber, and can be rethrown by the caller
- fiber printer
- supports any callable object
- built-in scheduler
- fast, very lightweight

## To build
```bash
git clone git@github.com:fix8mt/f8fiber.git
cd f8fiber
mkdir build
cd build
cmake ..
make -j4
```

## Runtime Options

### Exceptions
Any exceptions caught within a fiber should be assigned to a `std::exception_ptr` using `std::current_exception`. The calling function should then rethrow using
`std::rethrow_exception`, as in the following example:

```c++
std::exception_ptr _eptr;

// in my_fiber
try
{
   .
   .
   .
}
catch (...)
{
   _eptr = std::current_exception();
}
.
.
.
// in caller
try
{
   this_fiber::yield();
   if (_eptr)
      std::rethrow_exception(std::exchange(_eptr, nullptr));
}
catch (const std::exception& e)
{
   std::cout << e.what() << '\n';
}
```
### Printer
You can print an `fiber`. The printer will print the `fiber_id`, the raw pointer to the resource object and a pointer to the typing object:
```c++
.
.
.
std::cout << my_fiber << endl;
.
.
.
```
Will produce something like this:
```
0x7f46a6a94df0 (0x7f46a6a94f00,0x555b69e2ad60)
```


## Example
```c++
#include <iostream>
#include <iomanip>
#include <thread>
#include <string>
#include <fix8/f8fiber.hpp>

using namespace FIX8;

class foo
{
   int _cnt{};

public:
   foo(int cnt) : _cnt(cnt) {}

   void func (bool& flags)
   {
      std::cout << "\tfunc:entry\n";
      std::cout << "\tcaller id:" << f.get_id() << '\n';
      for (int kk{}; kk < _cnt; ++kk)
      {
         std::cout << "\tfunc:" << kk << '\n';
         this_fiber::yield();
         std::cout << "\tfunc:" << kk << " (resumed)\n";
      }
      flags = true;
      std::cout << "\tfunc:exit\n";
   }
};

int main(int argc, char *argv[])
{
   bool flags{};
   foo bar(argc > 1 ? std::stol(argv[1]) : 5);
   fiber f0(&foo::func, &bar, std::ref(flags));
   std::cout << "fiber id:" << f0.get_id() << '\n';
   std::cout << "flags=" << std::boolalpha << flags << '\n';

   for (int ii{}; f0; ++ii)
   {
      std::cout << "main:" << ii << '\n';
      this_fiber::yield();
      std::cout << "main:" << ii << " (resumed)\n";
   }
   std::cout << "flags=" << std::boolalpha << flags << '\n';
   std::cout << "main:exit\n";
   return 0;
}
```
### Output
```bash
% ./f8fibertest1
fiber id:0x7fdcce843f50
flags=false
main:0
        func:entry
        caller id:0x7ffcd6370b30
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
        func:exit
main:5 (resumed)
flags=true
main:exit
%
```
