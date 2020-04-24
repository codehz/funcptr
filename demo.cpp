#include <functional>
#include <iostream>

#include "include/funcptr.hpp"

using namespace func_ptr;

void fake_clib_func(void (*callback)(int input, void *user), void *user) {
  std::cout << "before" << std::endl;
  callback(42, user);
  std::cout << "after" << std::endl;
}
void another_clib_func(void (*callback)(void *user, int input), void *user) {
  std::cout << "[another] before" << std::endl;
  callback(user, 233);
  std::cout << "[another] after" << std::endl;
}

void test(int input, user_pointer) {
  std::cout << "got " << input << std::endl;
}

int main() {
  funcptr fn{test};
  fake_clib_func(fn, fn.holder());

  funcptr fnobj{std::function<void(int, user_pointer)>{test}};
  fake_clib_func(fnobj, fnobj.holder());

  int binding;

  funcptr lambda{[&](int input, user_pointer) { binding = input; }};
  fake_clib_func(lambda, lambda.holder());

  std::cout << "[main] got " << binding << std::endl;

  funcptr another_lambda{[&](user_pointer, int input) { binding = input; }};
  another_clib_func(another_lambda, another_lambda.holder());

  std::cout << "[main] got " << binding << std::endl;
}