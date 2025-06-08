#include "simplelog/simplelog.hpp"

struct Point
{
  int x, y, z;
};

static int sum_xy(const Point &p)
{
  return p.x + p.y;
}

int main()
{
  Point p {.x = 5, .y = 6, .z = 7 };
  LOG << "sum struct " << sum_xy(p);

  return 0;
}
