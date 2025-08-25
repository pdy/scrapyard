#include "simplelog/simplelog.hpp"
//#include <memory>

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

struct StackUnwind
{
  size_t count {0};

  ~StackUnwind()
  {
    LOG << "Unwind " << count;
  }
};

static void throw_exception()
{
  StackUnwind s1 { 5 };
  StackUnwind s2 { 6 };
  LOG << "Throwing exception String";
//  throw "String";
  StackUnwind s3 { 7 };
  StackUnwind s4 { 8 };
}

int main()
{

  StackUnwind s0 { 0 };
  {
    StackUnwind s1 { 1 };
    StackUnwind s2 { 2 };
    StackUnwind s3 { 3 };
    StackUnwind s4 { 4 };
  }
  StackUnwind s7 { 7 };
  /*
  try{
    throw_exception();
    StackUnwind s3 { 3 };
    StackUnwind s4 { 4 };
  }
  catch(const char *str)
  {
    LOG << "Catched exception " << str;
  }
*/
  return 0;
}
