#include <functional>
#include <iostream>

#include "include/hackptr.hpp"

using namespace func_ptr;

void fake_clib_func(void (*callback)(int input)) {
  std::cout << "before" << std::endl;
  callback(42);
  std::cout << "after" << std::endl;
}

int main() {
  hackfunc fn{
      [](int input) { std::cout << "directly: " << input << std::endl; }};
  
  fake_clib_func(fn);

  int binding;

  hackfunc fn2{
      [&](int input) { binding = input; }};
  
  fake_clib_func(fn2);
  std::cout << "[main] got " << binding << std::endl;
}