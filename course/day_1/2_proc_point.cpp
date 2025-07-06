#include "simplelog/simplelog.hpp"


enum class Status
{
  Ok,
  Error
};

struct Point
{
  int x, y, z;

  Status status;

  explicit operator bool() const { return status == Status::Ok; }
};

static int sum_xy(const Point &p)
{
//  char *str = new char[5];
//


  return p.x + p.y;
}


static Point returnPoint()
{
  return Point{.x = 5, .y = 6, .z = 7, .status = Status::Ok };
}

auto returnPointTuple()
{
  return std::make_tuple(5, 6, 7);
}

// C++17 structured bindings

int main()
{
  Point p {.x = 5, .y = 6, .z = 7 };


  const auto [x, y, z, status] = returnPoint();


  if(Point p3 = returnPoint())
  {

  }
  else
  {
    p3.status;
  }

  LOG << "x " << x;


//  LOG << "sum struct " << sum_xy(p);


  return 0;
}
