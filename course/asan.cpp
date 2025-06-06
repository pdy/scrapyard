#include <iostream>

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

  std::cout << *ref << '\n';

//  auto * ptr = new int;
  return 0;
}
