#include <iterator>
#include <vector>
#include <algorithm>
#include <string>

struct Type
{
  int i;
  double d;
  std::string str;
};

int main()
{
  using T = Type;

  std::vector<T> from(1000000);

  std::vector<T> to; to.reserve(from.size());

  std::copy(from.begin(), from.end(), std::back_inserter(to));

  return 0;
}
