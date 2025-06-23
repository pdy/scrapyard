#include "simplelog/simplelog.hpp"


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
  throw "String";
  StackUnwind s3 { 7 };
  StackUnwind s4 { 8 };
}

int main()
{
  StackUnwind s1 { 1 };
  StackUnwind s2 { 2 };
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
