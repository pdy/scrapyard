#include "simplelog/simplelog.hpp"

static void func(int **p)
{
  int scoped = 6;
  *p = &scoped;
}

int main()
{
#if 0 
  int var = 5;
  int *ref = &var;

  func(&ref);

  LOG << "ref " << *ref;
#endif
    auto * ptr = new int;
  return 0;
}
