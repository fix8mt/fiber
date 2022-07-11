<p align="center">
  <a href="https://www.fix8mt.com"><img src="assets/fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# f8fiber
### C++17 fiber based on modified `boost::fiber`, fcontext / x86_64 / linux only / de-boosted

------------------------------------------------------------------------

This is a modified and stripped down version of [boost::fiber](https://www.boost.org/doc/libs/release/libs/fiber/), with the main differences as follows:
- x86_64 Linux only
- fcontext inline assembly
- stack uses mmap
- no custom allocator support, fiber record uses heap
- simplified API, rvalue and lvalue resume()
- supports any callable object
- _de-boosted_, no boost dependencies
- no scheduler, no boost::context
- minimal static lib, the rest in header
- fast, lightweight

