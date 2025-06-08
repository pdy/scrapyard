#include "simplelog/simplelog.hpp"

class Point
{
  int m_x, m_y, m_z;

public:
  Point(int x, int y, int z)
    : m_x{x}, m_y{y}, m_z{z}
  {}

  int sum_xy() const { return m_x + m_y; }
};

int main()
{
  Point p(5, 6, 7);
  LOG << "sum class " << p.sum_xy();

  return 0;
}
