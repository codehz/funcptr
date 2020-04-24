#pragma once

#include <cstring>
#include <functional>

// clang-format off
#if defined(__x86_64__) || defined(_M_AMD64)
#  define FUNCPTR_ARCH_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#  define FUNCPTR_ARCH_X86
#else
#  error Not support target arch
#endif

#if defined(__linux__)
#  define FUNCPTR_OS_LINUX
#elif defined(_WIN32)
#  define FUNCPTR_OS_WINDOWS
#else
#  error Not support target os
#endif

#if defined(__GNUC__)
#  define FUNCPTR_COMPILER_GCC
#elif defined(_MSC_VER)
#  define FUNCPTR_COMPILER_MSVC
#else
#  error Not support compiler
#endif

#if defined(FUNCPTR_COMPILER_GCC)
#  define FUNCPTR_PACK_BEFORE
#  define FUNCPTR_PACK_AFTER __attribute__((packed))
#elif defined(FUNCPTR_COMPILER_MSVC)
#  define FUNCPTR_PACK_BEFORE __pragma(pack(push, 1))
#  define FUNCPTR_PACK_AFTER ;__pragma(pack(pop))
#endif

#if defined(FUNCPTR_OS_LINUX)
#  include <sys/mman.h>
#elif defined(FUNCPTR_OS_WINDOWS)
#  include <windows.h>
#endif
// clang-format on

namespace func_ptr {

namespace details {

template <typename ret, typename... params> struct hackfunc_impl;

#if defined(FUNCPTR_ARCH_X86_64)

FUNCPTR_PACK_BEFORE
template <typename ret, typename... params> struct alignas(8) hackfunc_impl {
  short mov_r10 = 0xba49;
  std::function<ret(params...)> *func;
  short mov_rax = 0xb848;
  void *ptr = (void *)wrapper;
  short jmp = 0xe0ff;

  hackfunc_impl(std::function<ret(params...)> &fn) : func(&fn) {}

  static auto wrapper(params... ps) {
    std::function<ret(params...)> *func;
    asm("movq %%r10, %0" : "=r"(func));
    return (*func)(ps...);
  }
} FUNCPTR_PACK_AFTER;

#else
#error TODO
#endif

void *alloc_executable_memory();

#if defined(FUNCPTR_OS_LINUX)

void *alloc_executable_memory() {
  return mmap(0, sizeof(hackfunc_impl<void, int>),
              PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS,
              -1, 0);
}

#elif defined(FUNCPTR_OS_WINDOWS)

void *alloc_executable_memory() {
  return VirtualAlloc(nullptr, sizeof(hackfunc_impl<void, int>), MEM_COMMIT,
                      PAGE_EXECUTE_READWRITE);
}

#else
#error TODO
#endif

} // namespace details

template <typename T, typename = void> struct hackfunc;

template <typename ret, typename... params>
struct hackfunc<std::function<ret(params...)>> {
  using proto = ret (*)(params...);
  using input = std::function<ret(params...)>;

  hackfunc(input fn) : storage(fn) {}

  operator proto() {
    auto temp = new (details::alloc_executable_memory())
        details::hackfunc_impl<ret, params...>(storage);
    proto swap;
    memcpy(&swap, &temp, sizeof swap);
    return swap;
  }

private:
  input storage;
};

template <typename lambda>
struct hackfunc<lambda,
                std::void_t<decltype(std::function(std::declval<lambda>()))>>
    : hackfunc<decltype(std::function(std::declval<lambda>()))> {
  hackfunc(lambda lam)
      : hackfunc<decltype(std::function(std::declval<lambda>()))>(lam) {}
};

template <typename ret, typename... params>
hackfunc(std::function<ret(params...)>)
    -> hackfunc<std::function<ret(params...)>>;

template <typename lambda> hackfunc(lambda lam) -> hackfunc<lambda>;

} // namespace func_ptr