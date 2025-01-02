#include <ranges>
#include <span>
#include <algorithm>
#include <concepts>

#include "simplelog/simplelog.h"
//#include "cmdline.h"

static void printView(std::span<int> ints)
{
}

int main()
{
  
  std::vector nums = {1, 5, 9, 3, 1, 50, 49};
  log << "nums " << nums;

  std::ranges::sort(nums);

  log << "nums sorted " << nums; 
  log << "any 1 " << (std::ranges::any_of(nums, [](int num) { return num == 1;}) ? "true" : "false");

  // this stuff does not work in clang 12
  auto reversed = nums | std::views::reverse;
  auto reversed_2 = std::ranges::reverse_view(nums);


  log << "reverse view types equal [" << (typeid(reversed) == typeid(reversed_2) ? "true" : "false") << "]";

  std::cout << "nums reversed ";
  for(const auto &el : reversed)
    std::cout << el << " ";

  std::cout << "\nnums reversed_2 ";
  for(const auto &el : std::ranges::reverse_view(nums))
    std::cout << el << " ";

  std::cout<< "\n";

  log << "sizeof void* [" << sizeof(void*) << "]\n";

  return 0;
}
