/*
*  MIT License
*  
*  Copyright (c) 2020 Pawel Drzycimski
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*
*/

#include <cmdline.h> 
#include <concepts>
#include <sstream>
#include "simplelog/simplelog.h"

static std::vector<int> toArr(std::string_view arrStr, char delim = ' ')
{
  const auto toInt = [](std::string_view str)
  {
    std::stringstream ss;
    ss << str;

    int ret;
    ss >> ret;

    return ret;
  };

  std::vector<int> ret; 

  size_t start = 0, end = 0;
  while((end = arrStr.find(delim, start)) != std::string_view::npos)
  {
    ret.push_back(toInt(arrStr.substr(start, end - start)));

    start = end + 1;
  }

  ret.push_back(toInt(arrStr.substr(start)));

  return ret;
}

static std::vector<long> calcPrefSum(const std::vector<int> &in)
{
  if(in.empty())
    return {};


  std::vector<long> ret;
  ret.reserve(in.size());

  ret.push_back(in[0]);
  for(size_t i = 1; i < in.size(); ++i)
  {
    ret.push_back(ret.back() + in[i]);
  }

  return ret;
}

static std::vector<int> calcReversePrefSum(const std::vector<int> &in)
{
  if(in.empty())
    return {};

  std::vector<int> ret(in.size());

  ret.back() = in.back();
  auto retBack = ret.rbegin();
  for(auto it = std::next(in.rbegin()); it != in.rend(); it = std::next(it), retBack = std::next(retBack))
  {
    *std::next(retBack) = *it + *retBack;
  }

  return ret;
}

template<std::integral T>
static void printCout(const std::vector<T> &arr)
{
  for(size_t i = 0; const auto num : arr)
  {
    std::cout << num;
    if(i < arr.size() - 1)
      std::cout << ' ';

    ++i;
  }
}

int main(int argc, char *argv[])
{
  /*
   *  Array equilibrium is index, where sum of the elements before and after are equal.
   *
   *  Solution to find this index is to calculate two prefix sum arrays,
   *  one from the begining and the other one from the end.
   *
   *  If in those two prefix sum arrays we find same value at the same index, then this
   *  index is our solution.
   *
   *
   */


  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
  arg.add<std::string>("input", 'i', "Space separated integers", true);
    
  if(!arg.parse(argc, const_cast<const char* const*>(argv)))
  {
    const auto fullErr = arg.error_full();
    if(!fullErr.empty())
      LOG << fullErr;
     
    LOG << arg.usage();
    return 0;
  }
  
  if(arg.exist("help"))
  {
    LOG << arg.usage();
    return 0;
  } 

  if(!arg.exist("input"))
  {
    LOG << "--input or -i argument is mandatory!\n";
    LOG << arg.usage();
    return 0;
  }
  
  const auto inArr = toArr(arg.get<std::string>("input"));
  const auto prefSum = calcPrefSum(inArr);
  const auto reversePrefSum = calcReversePrefSum(inArr);

  std::cout << "             input [";
  printCout(inArr);
  std::cout << "]\n";

  std::cout << "        prefix sum [";
  printCout(prefSum);
  std::cout << "]\n";

  std::cout << "reverse prefix sum [";
  printCout(reversePrefSum);
  std::cout << "]\n";

  bool atLeastOne = false;
  for(size_t i = 0; i < prefSum.size(); ++i)
  {
    if(prefSum[i] == reversePrefSum[i])
    {
      LOG << "found at index [" << i << "] val [" << inArr[i] << "]";
      atLeastOne = true;
    }
  }
  
  if(!atLeastOne)
    LOG << "not found";  
   
  return 0;
}
