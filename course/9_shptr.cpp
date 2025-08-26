#include "simplelog/simplelog.hpp"
#include <memory>

struct C;

struct A
{
  std::weak_ptr<C> ptr;
};

struct B
{
  std::weak_ptr<A> ptr;
};

struct C
{
  std::weak_ptr<B> ptr;
};

struct Type
{
  int integer{0};
  float floatNum{.0};
};

static void use_ptr(std::shared_ptr<Type> sharg)
{
  LOG << "shptr use_count: " << sharg.use_count()<< " is unique " << sharg.unique();
}

static void use_weak_ptr(std::weak_ptr<Type> weak)
{
  LOG << "weak_ptr use_count: " << weak.use_count();
  if(std::shared_ptr<Type> shptr = weak.lock())
  {
    LOG << "LOCKED weak_ptr use_count: " << weak.use_count();
  }
  LOG << "weak_ptr use_count: " << weak.use_count();
}

int main()
{
  Type *ptr {nullptr};
  LOG << "sizeof(ptr): " << sizeof(ptr);

  std::shared_ptr<Type> shptr{  };
  LOG << "sizeof(shptr): " << sizeof(shptr);

  LOG << "shptr use_count: " << shptr.use_count() << " is unique " << shptr.unique();
  use_weak_ptr(shptr);

  return 0;
}
