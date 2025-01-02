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

struct TestCase
{
  std::string msg;
  unsigned k;
  std::string expected;
};

static std::string ceasar(std::string msg, unsigned k)
{
  constexpr unsigned charsCount = 'z' + 1 - 'a';
  const unsigned rot = k % charsCount;

  for(size_t i = 0; const char c : msg)
  {
    if(c >= 'a' && c <='z')
    {
      msg[i] = (c + rot) % 'z';
      if(msg[i] == 0)
        msg[i] = 'z';
      else if(msg[i] < 'a')
        msg[i] += 'a' - 1;
    }
    else if(c >='A' && c <='Z')
    {
      msg[i] = (c + rot) % 'Z';
      if(msg[i] == 0)
        msg[i] = 'Z';
      else if(msg[i] < 'A')
        msg[i] += 'A' - 1;
    }

    ++i;
  }

  return msg;
}

int main()
{
  
  const TestCase TESTS[] = {
    TestCase{
      .msg = "xyz",
      .k = 3,
      .expected = "abc"
    },
    
    TestCase{
      .msg = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
      .k = 3,
      .expected = "DdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsUuVvWwXxYyZzAbCc"
    },

    TestCase{
      .msg = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
      .k = 26,
      .expected = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
    },

    TestCase{
      .msg = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
      .k = 0,
      .expected = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
    },

    TestCase{
      .msg = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
      .k = 52,
      .expected = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
    },

    TestCase{
      .msg = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
      .k = 53,
      .expected = "BbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqSsTtUuVvWwXxYzAa"
    },

    TestCase{
      .msg = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpRrSsTtUuVvWwXyZz",
      .k = 1,
      .expected = "BbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqSsTtUuVvWwXxYzAa"
    }
  };


  for(const auto &test : TESTS)
  {
    const auto actual = ceasar(test.msg, test.k);
    LOG << "msg       [" << test.msg << "] rot [" << test.k << "]";
    LOG << "exepected [" << test.expected << "]";
    LOG << "actual    [" << actual << "]";
    LOG << std::string(70, '-') << (actual == test.expected ? "Ok" : "FAIL!");
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
