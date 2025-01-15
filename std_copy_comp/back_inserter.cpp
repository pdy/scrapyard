#include <iterator>
#include <vector>
#include <algorithm>
#include <string>

#include "simplelog/simplelog.h"

struct Type
{
  int i;
  double d;
  std::string str;
};

int main()
{
  using T = Type;

  std::vector<T> from(10000000);

  std::vector<T> to; to.reserve(from.size());

  const auto now = NOW();
  std::copy(from.begin(), from.end(), std::back_inserter(to));
  LOG << DURATION_MS(now).count();

  return 0;
}
