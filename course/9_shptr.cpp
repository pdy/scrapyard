#include "simplelog/simplelog.hpp"
#include <memory>

struct Type
{
  int integer{0};
  float floatNum{.0};
};


static void use_ptr(std::shared_ptr<Type> sharg)
{
  LOG << "shptr use_count: " << sharg.use_count()<< " is unique " << sharg.unique() ;
}

int main()
{
  Type *ptr {nullptr};
  LOG << "sizeof(ptr): " << sizeof(ptr);

  std::shared_ptr<Type> shptr{new Type};
  LOG << "sizeof(shptr): " << sizeof(shptr);
  LOG << "shptr use_count: " << shptr.use_count() << " is unique " << shptr.unique();
  use_ptr(shptr);

  return 0;
}
