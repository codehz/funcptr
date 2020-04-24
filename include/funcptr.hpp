#pragma once

#include <cstring>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace func_ptr {

struct user_pointer {
  void *value;

  user_pointer(void *value) : value(value) {}

  template <typename target> inline target *cast() {
    target *ret;
    memcpy(&ret, &value, sizeof value);
    return ret;
  }
};

namespace details {

template <class... T> struct always_false : std::false_type {};

template <typename src> struct user_pointer_transform { using type = src; };
template <> struct user_pointer_transform<user_pointer> {
  using type = void *;
};

template <typename src>
using user_pointer_transform_t = typename user_pointer_transform<src>::type;

template <typename ret, typename... params>
using funcproto_transform_t = ret (*)(user_pointer_transform_t<params>...);

template <typename... params> struct has_user_pointer;

template <> struct has_user_pointer<> : std::false_type {};
template <> struct has_user_pointer<user_pointer> : std::true_type {};
template <typename... rest>
struct has_user_pointer<user_pointer, rest...> : std::true_type {};
template <typename first, typename... rest>
struct has_user_pointer<first, rest...> : has_user_pointer<rest...> {};

template <typename... params>
inline constexpr bool has_user_pointer_v = has_user_pointer<params...>::value;

template <typename... params> struct user_pointer_trait;

template <typename... rest> struct user_pointer_trait<user_pointer, rest...> {
  inline static user_pointer get_user_pointer(user_pointer ptr, rest...) {
    return ptr;
  }
};
template <typename first, typename... rest>
struct user_pointer_trait<first, rest...> {
  inline static user_pointer get_user_pointer(first, rest... args) {
    return user_pointer_trait<rest...>::get_user_pointer(args...);
  }
};
template <> struct user_pointer_trait<user_pointer> {
  inline static user_pointer get_user_pointer(user_pointer ptr) { return ptr; }
};
template <typename no> struct user_pointer_trait<no> {
  static_assert(always_false<no>::value,
                "user_pointer not appear in the argument list");
};
} // namespace details

template <typename proto, typename = void> struct funcptr;

template <typename ret, typename... params> struct funcptr<ret (*)(params...)> {
  static_assert(details::has_user_pointer_v<params...>,
                "Require at least one user_pointer argument");
  using proto = details::funcproto_transform_t<ret, params...>;
  using input = ret (*)(params...);
  inline constexpr explicit funcptr(input src) : src(src) {}
  inline constexpr operator proto() const {
    return +[](details::user_pointer_transform_t<params>... ps) -> ret {
      user_pointer ptr =
          details::user_pointer_trait<params...>::get_user_pointer(ps...);
      return (ptr.cast<ret(params...)>())(std::forward<decltype(ps)>(ps)...);
    };
  }
  inline constexpr void *holder() const { return (void *)src; }

private:
  input src;
};

template <typename ret, typename... params>
struct funcptr<std::function<ret(params...)>> {
  static_assert(details::has_user_pointer_v<params...>,
                "Require at least one user_pointer argument");
  using proto = details::funcproto_transform_t<ret, params...>;
  using input = std::function<ret(params...)>;
  inline explicit funcptr(input &&src) : storage(new input(src)) {}
  inline operator proto() {
    return [](details::user_pointer_transform_t<params>... ps) -> ret {
      user_pointer ptr =
          details::user_pointer_trait<params...>::get_user_pointer(ps...);
      return (*ptr.cast<input>())(std::forward<decltype(ps)>(ps)...);
    };
  }
  inline void *holder() { return storage.get(); }

private:
  std::unique_ptr<input> storage;
};

template <typename lambda>
struct funcptr<lambda,
               std::void_t<decltype(std::function(std::declval<lambda>()))>>
    : funcptr<decltype(std::function(std::declval<lambda>()))> {
  funcptr(lambda lam)
      : funcptr<decltype(std::function(std::declval<lambda>()))>(lam) {}
};

template <typename ret, typename... params>
funcptr(ret (*)(params...)) -> funcptr<ret (*)(params...)>;

template <typename ret, typename... params>
funcptr(std::function<ret(params...)>)
    -> funcptr<std::function<ret(params...)>>;

template <typename lambda> funcptr(lambda lam) -> funcptr<lambda>;

} // namespace func_ptr