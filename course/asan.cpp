#include "simplelog/simplelog.hpp"

static void func(int **p)
{
  int scoped = 6;
  *p = &scoped;
}

int main()
{
  
  int var = 5;
  int *ref = &var;
  func(&ref);

  LOG << "ref " << *ref;

  //  auto * ptr = new int;
  return 0;
}
