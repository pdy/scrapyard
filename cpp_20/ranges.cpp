#include <ranges>
#include <algorithm>
#include <concepts>

#include "simplelog/simplelog.h"
#include "cmdline.h"

int main()
{
  
  std::vector nums {1, 5, 9, 3, 1, 50, 49};
  log << "nums " << nums;

  std::ranges::sort(nums);

  log << "nums sorted " << nums; 
  
  return 0;
}
