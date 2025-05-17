#include <cmath>
#include <memory>
#include <type_traits>
#include <concepts>

#include "simplelog/simplelog.h"

struct Type
{
  int placeholder{0};

  ~Type() {}
};

template<typename T>
concept has_placeholder = requires {T::placeholder;};

static void onlyWithPlaceholder(const has_placeholder auto &var)
{
  log << "placeholder: " << var.placeholder;
}

static size_t powerOfTwo(std::integral auto num)
{
  return num * num;
}

int main()
{
  Type t { 5 };
  onlyWithPlaceholder(t);

  log << "pow2: " << powerOfTwo(2);


  return 0;
}
