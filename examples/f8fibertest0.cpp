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
		std::cout << "\tfunc:entry (fiber id:" << this_fiber::get_id() << ")\n";
      for (int kk{}; kk < _cnt; ++kk)
      {
         std::cout << "\tfunc:" << kk << '\n';
         this_fiber::yield();
         std::cout << "\tfunc:" << kk << " (resumed)\n";
      }
      flags = true;
		fibers::print();
      std::cout << "\tfunc:exit\n";
   }
};

int main(int argc, char *argv[])
{
   bool flags{};
   foo bar(5);
   fiber f0({.name="foo::func"}, &foo::func, &bar, std::ref(flags));
   std::cout << "flags=" << std::boolalpha << flags << '\n';

   for (int ii{}; fibers::has_fibers(); ++ii)
   {
      std::cout << "main:" << ii << '\n';
      this_fiber::yield();
      std::cout << "main:" << ii << " (resumed)\n";
   }
   std::cout << "flags=" << std::boolalpha << flags << '\n';
   std::cout << "main:exit\n";
   return 0;
}
