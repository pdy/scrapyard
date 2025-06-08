#include <iostream>

struct Point
{
  int x{0}, y{0}, z{0};
};

static int sum_xy(const Point &p)
{
  return p.x + p.y;
}

int main()
{
  Point def;
  std::cout << "sum def struct " << sum_xy(def) << '\n';

  Point p {.x = 5, .y = 6, .z = 7 };
  std::cout << "sum struct " << sum_xy(p) << '\n';

  return 0;
}
