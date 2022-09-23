//-----------------------------------------------------------------------------------------
// f8_sched_fiber (header only)
// Copyright (C) 2022 Fix8 Market Technologies Pty Ltd
// see https://github.com/fix8mt/f8fiber
//
// Lightweight header-only stackful per-thread fiber with built-in roundrobin scheduler
//
// Distributed under the Boost Software License, Version 1.0 August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//----------------------------------------------------------------------------------------
#ifndef FIX8_SCHEDFIBER_HPP_
#define FIX8_SCHEDFIBER_HPP_
#include <iostream>
#include <type_traits>
#include <queue>
#include <chrono>
#include <algorithm>
#include <functional>
#include <bitset>
#include <string_view>
#include <mutex>
#include <condition_variable>
#include <set>

//-----------------------------------------------------------------------------------------
#if !defined (SIGSTKSZ)
# define SIGSTKSZ 131072 // 128kb recommended
#endif

//-----------------------------------------------------------------------------------------
namespace FIX8 {

//-----------------------------------------------------------------------------------------
/// simple non-copyable base
#if !defined f8_noncopyable
class f8_noncopyable
{
protected:
	f8_noncopyable() = default;
	~f8_noncopyable() = default;

	f8_noncopyable(const f8_noncopyable&) = delete;
	f8_noncopyable(f8_noncopyable&&) = delete;
	f8_noncopyable& operator=(const f8_noncopyable&) = delete;
	f8_noncopyable& operator=(f8_noncopyable&&) = delete;
};
#endif

#if !defined f8_nonconstructible
/// simple non-constructable base
class f8_nonconstructible : protected f8_noncopyable
{
protected:
	f8_nonconstructible() = delete;
	~f8_nonconstructible() = delete;
};
#endif

//-----------------------------------------------------------------------------------------
/// unique fiber id
class f8_fiber_id
{
	const void *ptr_{ nullptr };

public:
	f8_fiber_id() = default;
	explicit f8_fiber_id(const void *ptr) noexcept : ptr_{ ptr } {}

#if __cplusplus >= 202002L
	constexpr auto operator<=>(const f8_fiber_id& other) const noexcept { return ptr_ <=> other.ptr_; }
#else
	constexpr bool operator==(const f8_fiber_id& other) const noexcept { return ptr_ == other.ptr_; }
	constexpr bool operator!=(const f8_fiber_id& other) const noexcept { return ptr_ != other.ptr_; }
	constexpr bool operator<(const f8_fiber_id& other) const noexcept { return ptr_ < other.ptr_; }
	constexpr bool operator>(const f8_fiber_id& other) const noexcept { return other.ptr_ < ptr_; }
	constexpr bool operator<=(const f8_fiber_id& other) const noexcept { return !(*this > other); }
	constexpr bool operator>=(const f8_fiber_id& other) const noexcept { return !(*this < other); }
#endif

	template<typename charT, class traitsT>
	friend std::basic_ostream<charT, traitsT>& operator<<(std::basic_ostream<charT, traitsT>& os, const f8_fiber_id& what)
	{
		if (what.ptr_)
			return os << what.ptr_;
		return os << "not a fiber";
	}

	constexpr explicit operator bool() const noexcept { return ptr_ != nullptr; }
	constexpr bool operator!() const noexcept { return ptr_ == nullptr; }
};

//-----------------------------------------------------------------------------------------
class f8_this_fiber : protected f8_nonconstructible
{
public:
	static f8_fiber_id get_id() noexcept;
	static void yield() noexcept;
	template<typename Clock, typename Duration>
	static void sleep_until(std::chrono::time_point<Clock, Duration> const& sleep_time);
	template<typename Rep, typename Period>
	static void sleep_for(std::chrono::duration<Rep, Period> const& rel_time);
};

class f8_fibers : protected f8_nonconstructible
{
public:
	static int size() noexcept;
	static int size_ready() noexcept;
	static bool has_fibers() noexcept;
	static bool has_ready_fibers() noexcept;
	static void print(std::ostream& os) noexcept;
};

//-----------------------------------------------------------------------------------------
class alignas(16) f8_sched_fiber
{
	uint64_t *_stk; // top of f8_sched_fiber stack
	uint64_t *_stk_alloc; // allocated stack memory
	std::bitset<8> _flags;
	std::chrono::steady_clock::time_point _tp{};
	std::condition_variable _cv_join;
	std::mutex _join_mutex;

	enum FiberFlags { main, finished, suspended, count  };

	struct cvars
	{
		std::set<f8_sched_fiber *> _uniq;
		std::queue<f8_sched_fiber *> _sched;
		f8_sched_fiber *_curr;
	};
	static cvars& get_vars() noexcept
	{
		static thread_local f8_sched_fiber _def_fiber;
		static thread_local cvars _cvars{._uniq={&_def_fiber},._curr=&_def_fiber};
		return _cvars;
	}

	static void f8_sched_fiber_exit()
	{
		std::cout << "f8_sched_fiber has exited. Terminating application.\n";
		exit(0);
	}

	template<typename Fn>
	class callable_wrapper
	{
		typename std::decay_t<Fn> _func;

	public:
		callable_wrapper(Fn&& func) noexcept : _func(std::forward<Fn>(func)) {}
		~callable_wrapper() = default;

		void exec()
		{
			_func();
			auto& [un, sch, cur] { get_vars() };
			cur->_flags[FiberFlags::finished] = true;
			if (!sch.empty())
				f8_this_fiber::yield();
			f8_sched_fiber_exit();
		}
	};

	template<typename Wrapper>
	static void jumper(void *ptr) { static_cast<Wrapper*>(ptr)->exec(); }

	// asm stack switch routine
	static void _coroswitch(f8_sched_fiber *old, f8_sched_fiber *newer) noexcept;

	// default current (main) f8_sched_fiber
	f8_sched_fiber() noexcept : _flags(1 << FiberFlags::main) {}

public:
	template<typename Fn, typename... Args, std::enable_if_t<!std::is_bind_expression_v<Fn>,int> = 0>
	f8_sched_fiber(Fn&& func, Args&&... args) : f8_sched_fiber(std::bind(std::forward<Fn>(func), std::forward<Args>(args)...)) {}

	template<typename Fn, typename Wrapper = callable_wrapper<Fn>>
	f8_sched_fiber(Fn&& func, size_t stacksz=SIGSTKSZ) : _stk_alloc(new uint64_t[stacksz / sizeof(uint64_t)])
	{
		_stk = _stk_alloc + stacksz / sizeof(uint64_t) - 1; // top of stack
		*--_stk = reinterpret_cast<uint64_t>(jumper<Wrapper>);
		*--_stk = reinterpret_cast<uint64_t>(new (_stk_alloc) callable_wrapper(std::forward<Fn>(func)));
		std::fill(_stk -= 7, _stk, 0x0);
		auto& [un, sch, cur] { get_vars() };
		if (un.insert(this).second)
			sch.push(this);
	}

	f8_sched_fiber(f8_sched_fiber&&) = default;
	f8_sched_fiber& operator=(f8_sched_fiber&&) = default;
	f8_sched_fiber(const f8_sched_fiber&) = delete;
	f8_sched_fiber& operator=(const f8_sched_fiber&) = delete;

	~f8_sched_fiber()
	{
		if (!_flags[FiberFlags::main])
		{
			delete[] _stk_alloc;
			if (joinable())
				f8_sched_fiber_exit();
		}
	}

	void join()
	{
		if (auto& cur { get_vars()._curr }; this != cur)
		{
			std::unique_lock jlock(_join_mutex);
			_cv_join.wait(jlock, [this]{ return !joinable();});
		}
	}
	bool joinable() const noexcept { return !_flags[FiberFlags::finished]; };

	explicit operator bool() const noexcept { return joinable(); }
	bool operator! () const noexcept { return !joinable(); }

	f8_fiber_id get_id() const noexcept { return f8_fiber_id(!_flags[FiberFlags::main] ? this : nullptr); }

	friend std::ostream& operator<<(std::ostream& os, const f8_sched_fiber& what)
	{
		return os << &what << (&what == f8_sched_fiber::get_vars()._curr ? "* " : " ") << what.get_id() << ' ' << what._flags;
	}
	friend f8_this_fiber;
	friend f8_fibers;
};

//-----------------------------------------------------------------------------------------
f8_fiber_id f8_this_fiber::get_id() noexcept { return f8_sched_fiber::get_vars()._curr->get_id(); }
void f8_this_fiber::yield() noexcept
{
	auto& [un, sch, cur] { f8_sched_fiber::get_vars() };
	while (!sch.empty())
	{
		auto& front { sch.front() };
		sch.pop();
		if (!front->_flags[f8_sched_fiber::FiberFlags::finished])
		{
			if (front->_flags[f8_sched_fiber::FiberFlags::suspended])
			{
				if (front->_tp < std::chrono::steady_clock::now())
				{
					front->_flags[f8_sched_fiber::FiberFlags::suspended] = false;
					front->_tp = decltype(front->_tp)();
				}
				else
				{
					sch.push(front);
					continue;
				}
			}
			std::swap(front, cur);
			sch.push(front);
			f8_sched_fiber::_coroswitch(front, cur);
			break;
		}
		else
			un.erase(front);
	}
}
template<typename Clock, typename Duration>
void f8_this_fiber::sleep_until(std::chrono::time_point<Clock, Duration> const& sleep_time)
{
	auto& cur { f8_sched_fiber::get_vars()._curr };
	cur->_tp = std::chrono::steady_clock::now() + (sleep_time - Clock::now());
	cur->_flags[f8_sched_fiber::FiberFlags::suspended] = true;
	yield();
}
template<typename Rep, typename Period>
void f8_this_fiber::sleep_for(std::chrono::duration<Rep, Period> const& rel_time)
{
	auto& cur { f8_sched_fiber::get_vars()._curr };
	cur->_tp = std::chrono::steady_clock::now() + rel_time;
	cur->_flags[f8_sched_fiber::FiberFlags::suspended] = true;
	yield();
}

//-----------------------------------------------------------------------------------------
int f8_fibers::size() noexcept { return f8_sched_fiber::get_vars()._sched.size(); }
int f8_fibers::size_ready() noexcept
{
	auto& [un, sch, cur] { f8_sched_fiber::get_vars() };
	return std::count_if(un.cbegin(), un.cend(), [&cur](f8_sched_fiber *pp)
		{ return pp != cur && !pp->_flags[f8_sched_fiber::FiberFlags::suspended] && !pp->_flags[f8_sched_fiber::FiberFlags::finished];});
}
bool f8_fibers::has_fibers() noexcept { return size(); }
bool f8_fibers::has_ready_fibers() noexcept { return size_ready(); }
void f8_fibers::print(std::ostream& os) noexcept
{
	for (const auto& pp : f8_sched_fiber::get_vars()._uniq)
		os << *pp << std::endl;
}

//-----------------------------------------------------------------------------------------
// static void f8_sched_fiber::_coroswitch(f8_sched_fiber *old, f8_sched_fiber *newer) noexcept;
asm(R"(.text
.align 16
.type _ZN4FIX814f8_sched_fiber11_coroswitchEPS0_S1_,@function
_ZN4FIX814f8_sched_fiber11_coroswitchEPS0_S1_:
	subq $0x40,%rsp
   stmxcsr (%rsp)
   fnstcw  4(%rsp)
	movq %r15,8(%rsp)
	movq %r14,8*2(%rsp)
	movq %r13,8*3(%rsp)
	movq %r12,8*4(%rsp)
	movq %rbx,8*5(%rsp)
	movq %rbp,8*6(%rsp)
	movq %rdi,8*7(%rsp)

	mov %rsp,(%rdi)
	mov (%rsi),%rsp

   ldmxcsr (%rsp)
   fldcw  4(%rsp)
   movq 8(%rsp),%r15
   movq 8*2(%rsp),%r14
   movq 8*3(%rsp),%r13
   movq 8*4(%rsp),%r12
   movq 8*5(%rsp),%rbx
   movq 8*6(%rsp),%rbp
   movq 8*7(%rsp),%rdi
	movq 8*8(%rsp),%r8
	addq $0x48,%rsp
   jmp *%r8
.size _ZN4FIX814f8_sched_fiber11_coroswitchEPS0_S1_,.-_ZN4FIX814f8_sched_fiber11_coroswitchEPS0_S1_
)");

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
namespace this_fiber
{
	inline f8_fiber_id get_id() noexcept { return f8_this_fiber::get_id(); }
	inline void yield() noexcept { return f8_this_fiber::yield(); }
	template<typename Clock, typename Duration>
	inline void sleep_until(std::chrono::time_point<Clock, Duration> const& sleep_time) { return f8_this_fiber::sleep_until(sleep_time); }
	template<typename Rep, typename Period>
	inline void sleep_for(std::chrono::duration<Rep, Period> const& rel_time) { return f8_this_fiber::sleep_for(rel_time); }
}

namespace fibers
{
	inline int size() noexcept { return f8_fibers::size(); }
	inline int size_ready() noexcept { return f8_fibers::size_ready(); }
	inline bool has_fibers() noexcept { return fibers::size(); }
	inline bool has_ready_fibers() noexcept { return fibers::size_ready(); }
	inline void print(std::ostream& os) noexcept { f8_fibers::print(os); }
};

} // namespace FIX8

#endif // FIX8_SCHEDFIBER_HPP_
