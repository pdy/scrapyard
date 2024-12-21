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
#include "simplelog/simplelog.h"

#include <vector>


using Matrix = std::vector<std::vector<int>>;

struct TestCase { std::string_view desc; Matrix mt; long long expectedSum;};

static long long calcSubSum(const Matrix &mt)
{
  const size_t N = mt.size();
  long long sum = 0;
  for(size_t r = 0; r < N / 2; ++r)
  {
    for(size_t c = 0; c < N / 2; ++c)
    {
      sum += std::max({
          mt[r][c],                // pos
          mt[N - 1 - r][c],        // column reverse
          mt[r][N - 1 - c],        // row reverse
          mt[N - 1 - r][N - 1 - c] // corner 
      });
    }
  }

  return sum;
}

int main(int, char * [])
{

  const TestCase TESTS [] = {
    {
      "4x4 1 - 5",
      Matrix{
        { 1,  1,  2,  2},
        { 1,  1,  2,  2},
        { 2,  2,  5,  5},
        { 2,  2,  5,  5}
      },
      5 + 5 + 5 + 5
    },
    
    {
      "4x4 1 - 16",
      Matrix{
        { 1,  2,  3,  4},
        { 5,  6,  7,  8},
        { 9, 10, 11, 12},
        {13, 14, 15, 16}
      },
      16 + 15 + 12 + 11
    },
 
    {
      "2x2 1,2,5,6",
      Matrix{
        { 1,  2},
        { 5,  6}
      },
      6
    }
  }; 

  for(auto it = std::begin(TESTS); it != std::end(TESTS); it = std::next(it))
  {
    const auto calculated = calcSubSum(it->mt);
    LOG << it->desc << '\n'
     << "    result [" << calculated << "]\n"
     << "  expected [" << it->expectedSum << "]\n"
     << "------------------------" << (calculated == it->expectedSum ? "Ok" : "Fail!") << '\n';
  }

  return 0;
}

#if 0
int main(int argc, char *argv[])
{
  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
//  arg.add<std::string>("file", 'f', "Example file argument.", true);
    
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

  /* Example file arg check and get
  if(!arg.exist("file"))
  {
    LOG << "--file or -f argument is mandatory!\n";
    LOG << arg.usage();
    return 0;
  }
  
  const std::string file = arg.get<std::string>("file");
  */
       
  return 0;
}
#endif
