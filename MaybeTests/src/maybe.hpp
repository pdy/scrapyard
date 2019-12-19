#ifndef NEITHER_MAYBE_HPP
#define NEITHER_MAYBE_HPP

#include <memory>
#include <cassert>
#include <cstddef>
#include <type_traits>
#include "traits.hpp"

namespace neither {

template <class T> struct Maybe;

template <> struct Maybe<void> {};

template <class T> struct Maybe {

  using size_type = std::size_t;

  union {
    T value;
  };

  bool const hasValue;

  constexpr Maybe() : hasValue{false} {}

  constexpr Maybe(T const& value) :  value{value}, hasValue{true} {}
  constexpr Maybe(T&& value) :  value{std::move(value)}, hasValue{true} {}

  constexpr Maybe(Maybe<void>) : hasValue{false} {}


  constexpr Maybe(Maybe<T>&& o) : hasValue{o.hasValue}
  {
    if(o.hasValue)
      value(std::move(o.value)); 
  }

  /*
  constexpr Maybe(Maybe<T> const &o) : hasValue{o.hasValue} {
    if (o.hasValue) {
      new (&value)T(o.value);
    }
  }
  */

  ~Maybe() {
    if (hasValue) {
      value.~T();
    }
  }

  constexpr T get(T defaultValue) {
    return hasValue ? value : defaultValue;
  }

  constexpr T unsafeGet() {
    assert(hasValue && "unsafeGet must not be called on an empty Maybe");
    return value;
  }

  constexpr size_type size() const noexcept { return hasValue ? 1: 0; }
  
  constexpr bool empty() const noexcept { return !hasValue; }

  constexpr operator bool() const { return hasValue; }
};

}

#endif
