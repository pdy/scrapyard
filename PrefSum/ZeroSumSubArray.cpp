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

template<typename T>
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
 
  std::cout << "     input [";
  printCout(inArr);
  std::cout << "]\n";

  std::cout << "prefix sum [";
  printCout(prefSum);
  std::cout << "]\n";

  struct Range {size_t start, end;};
  std::vector<Range> zeroSumsSubArrays;

  for(size_t i = 0; i < prefSum.size(); ++i)
  {
    const long value = prefSum[i];
    if(i != 0 && value == 0)
    {
      zeroSumsSubArrays.push_back({.start = 0, .end = i});
    }
    else if(i < prefSum.size() - 1)
    {
      for(size_t j = i + 1; j < prefSum.size(); ++j)
      {
        if(value == prefSum[j])
        {
          zeroSumsSubArrays.push_back({.start = i + 1, .end = j});
          break;
        }
      }
    }
  }
  
  if(zeroSumsSubArrays.empty())
  {
    LOG << "No sub arrays which sums to zero";
  }
  else
  {
    std::cout << "Sub ranges: ";
    for(const auto &r : zeroSumsSubArrays)
    {
      std::cout << "[" << r.start << " - " << r.end << "] ";
    }
    std::cout << '\n';
  }
       
  return 0;
}
