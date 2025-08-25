#include "simplelog/simplelog.hpp"
#include <memory>

#if 0
template<typename T, auto Deleter>
struct GenericDeleter
{
  void operator()(T *rc) const
  {
    Deleter(rc);
  }
};

template<typename T, auto Func>
using legacy_ptr = std::unique_ptr<T, GenericDeleter<T, Func>>;
#endif

struct Type
{
  int integer{0};
  float floatNum{.0};
};


int main()
{
  Type *ptr {nullptr};
  LOG << "sizeof(ptr): " << sizeof(ptr);

  std::unique_ptr<Type> uptr;
  LOG << "sizeof(uptr): " << sizeof(uptr);

  return 0;
}
