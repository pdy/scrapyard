#include <iostream>

class Point
{
  int m_x{0}, m_y{0}, m_z{0};

public:
  Point(int x, int y, int z)
    : m_x{x}, m_y{y}, m_z{z}
  {}

  int sum_xy() const { return m_x + m_y; }
};

int main()
{
  Point p(5, 6, 7);
  std::cout << "sum class " << p.sum_xy() << '\n';

  return 0;
}
