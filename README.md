<p align="center">
  <a href="https://www.fix8mt.com"><img src="assets/fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# f8fiber
#### C++17 fiber implementation based on modified _boost::fiber_, fcontext / x86_64 / linux only / de-boosted

## Introduction
This is a modified version of boost::fiber. The main differences are as follows:
- x86_64 Linux fcontext only
- stack uses mmap
- no custom allocator, fiber record uses heap
- simplified API, rvalue and lvalue resume()
- deboosted, no boost dependencies
- no scheduler, no boost::context
- lightweight static lib, the rest inlined header

