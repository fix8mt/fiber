#include <iostream>
#include <iomanip>
#include <thread>
#include <string>
#include <fix8/f8fiber.hpp>

using namespace FIX8;

void func (bool& flags, int cnt)
{
	std::cout << "\tfunc:entry (fiber id:" << this_fiber::get_id() << ")\n";
	for (int kk{}; kk < cnt; ++kk)
	{
		std::cout << "\tfunc:" << kk << '\n';
		this_fiber::yield();
		std::cout << "\tfunc:" << kk << " (resumed)\n";
	}
	flags = true;
	fibers::print();
	std::cout << "\tfunc:exit\n";
}

int main(int argc, char *argv[])
{
   std::cout << "main:entry\n";
   bool flags{};
   fiber f0({.name="func"}, &func, std::ref(flags), 5);
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
