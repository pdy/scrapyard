#include <ranges>
#include <algorithm>
#include <concepts>

#include "simplelog/simplelog.h"
#include "cmdline.h"

int main()
{
  
  std::vector nums = {1, 5, 9, 3, 1, 50, 49};
  log << "nums " << nums;

  std::ranges::sort(nums);

  log << "nums sorted " << nums; 

  // this stuff does not work in clang 12
//  auto reversed = nums | std::views::reverse;
//  auto reversed_2 = std::ranges::reverse_view(nums);

  return 0;
}
