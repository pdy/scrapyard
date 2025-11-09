#include "simplelog/simplelog.hpp"
#include <memory>

template<typename T, auto Deleter>
struct GenericDeleter
{
  void operator()(T *rc) const
  {
    LOG << "generic deleter";
    Deleter(rc);
  }
};

template<typename T, auto Func>
using legacy_ptr = std::unique_ptr<T, GenericDeleter<T, Func>>;

struct Type
{
  int integer;
  float floatNum;
};

static Type* init()
{
  return new Type;
}

static void free_type(Type *type)
{
  LOG << "free_type";
  delete type;
}

static void free_type_2(Type *type)
{
}

struct FreeTypeFunctor
{
  void operator()(Type *type)
  {
    LOG << "typed deleter";
    free_type(type);
  }
};

using Type_uptr = std::unique_ptr<Type, decltype(&free_type)>;
using Type_uptr_2 = std::unique_ptr<Type, FreeTypeFunctor>;
using Type_uptr_3 = legacy_ptr<Type, free_type>;
using Type_uptr_4 = legacy_ptr<Type, free_type_2>;


static void use_uptr(Type_uptr uptr)
{
  LOG << "use_uptr";
}

static void use_type_by_ref(Type &ref)
{
  LOG << "use by ref";
}

int main()
{
  Type *ptr {nullptr};
  LOG << "sizeof(ptr): " << sizeof(ptr);

  Type_uptr uptr(new Type, &free_type);

  Type_uptr_2 uptr_2(new Type);
  Type_uptr_3 uptr_3(new Type);

  LOG << "sizeof(uptr): " << sizeof(uptr);
  LOG << "sizeof(uptr_2): " << sizeof(uptr_2);
  LOG << "sizeof(uptr_3): " << sizeof(uptr_3);
//  use_uptr(std::move(uptr)); 

  return 0;
}
