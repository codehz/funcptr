# FuncPtr

A bridge for convert c++ function object to c function pointer (with user pointer)

Have you ever lost your head in passing a function to the C interface? Are you still fighting against `void *` ? With funcptr, you no longer have to worry about passing C++ functions in the C interface

## Usage

```c++
  void some_c_interface(int arg, void(*callback)(int input, void *user), void *user);

  using namespace func_ptr;
  int captured;
  funcptr lambda{[&](int input /* Normal parameter */, user_pointer /* Use this for user pointer placeholder */) { process(input, captured); }};
  some_c_interface(arg /* Normal parameter */, lambda, lambda.holder() /* User pointer */);
```

It support

1. Pass normal function to c interface (Not recommanded, please pass it directly)
2. Pass std::function to c interface
3. Pass lambda and std::bind

## Lifetime

All funcptr

## TODO

1. Add a asm magic for pass c++ function to c interface that no user pointer.

## LICENSE

Unlicensed see [LICENSE](LICENSE);
