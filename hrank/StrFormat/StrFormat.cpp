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


static std::string formatStr(std::string_view str)
{
  constexpr char SPACE = ' ';
  constexpr char CHARS[] = {SPACE, '-'};

  std::string stripped;
  for(char chr : str)
  {
    if(std::any_of(std::begin(CHARS), std::end(CHARS), [&](char c) { return c == chr;}))
      continue;

    stripped.push_back(chr);
  }


  size_t blockOfThreeLen = stripped.size();
  size_t blockOfTwoCount = stripped.size() - blockOfThreeLen;
  while(blockOfThreeLen % 3 != 0 || blockOfTwoCount % 2 != 0)
  {
    --blockOfThreeLen;
    ++blockOfTwoCount;// = stripped.size() - blockOfThreeLen;
  }

  std::string ret;
  for(size_t i = 0; i < blockOfThreeLen; ++i)
  {
    if(i && i % 3 == 0)
      ret.push_back(' ');

    ret.push_back(stripped[i]);
  }


  if(blockOfTwoCount)
  {
    if(blockOfThreeLen)
      ret.push_back(SPACE);

    size_t count = 0;
    for(size_t i = blockOfThreeLen; i < stripped.size(); ++i, ++count)
    {
      if(i != blockOfThreeLen && count % 2 == 0)
        ret.push_back(' ');

      ret.push_back(stripped[i]);
    }
  } 


#if 0
  size_t blockSize = 3, blockCounter = 1;
  for(size_t i = 0; i < stripped.size(); ++i, ++blockCounter)
  {  
    ret.push_back(stripped[i]);


  
    if(blockCounter == blockSize && i < stripped.size() - 1)
    {
      ret.push_back(SPACE);
      blockCounter = 0;

    }

    
//    const auto remaining = stripped.size() - 1 - i;
//    if(remaining == blockSize + 1)
//      blockSize = 2;

  }


  auto preLast = std::prev(std::prev(ret.end()));
  if(*preLast == SPACE)
  {
    ret.erase(preLast, ret.end());

    auto last = std::prev(ret.end());
    --(--last);

    ret.insert(last, SPACE);
  }
#endif


  ret.push_back(';');
  return ret;
}


struct Test
{
  std::string_view input;
  std::string_view output;
};


int main(int /*argc*/, char * /*argv*/ [])
{
  
  const Test TESTS[] = {
    Test{
      .input = "AB",
      .output = "AB;"
    },
    Test{
      .input = "AB16",
      .output = "AB 16;"
    },
    Test{
      .input = "AB1",
      .output = "AB1;"
    },
    Test{
      .input = "AB1--56",
      .output = "AB1 56;"
    },

    Test{
      .input = "AB1--567",
      .output = "AB1 567;"
    },
    
    Test{
      .input = "AB123 56-56",
      .output = "AB1 235 656;"
    },
    
    Test{
      .input = "AB1-3 56-56",
      .output = "AB1 356 56;"
    },
    Test{
      .input = "AB123 56-566",
      .output = "AB1 235 65 66;"
    },
    Test{
      .input = "AB123 56-566---5656",
      .output = "AB1 235 656 656 56;"
    }
    
  };
 
  for(auto it = std::begin(TESTS); it != std::end(TESTS); it = std::next(it))
  {
    const auto result = formatStr(it->input);
    log << "input   [" << it->input << "]";
    log << "output  [" << result << "]";
    log << "correct [" << it->output << "]";
    log << "--------------------------------" << (it->output == result ? "OK!" : "FAIL!");
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
      log << fullErr;
     
    log << arg.usage();
    return 0;
  }
  
  if(arg.exist("help"))
  {
    log << arg.usage();
    return 0;
  } 

  /* Example file arg check and get
  if(!arg.exist("file"))
  {
    log << "--file or -f argument is mandatory!\n";
    log << arg.usage();
    return 0;
  }
  
  const std::string file = arg.get<std::string>("file");
  */
       
  return 0;
}
#endif
