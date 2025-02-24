/*
*  MIT License
*  
*  Copyright (c) 2025 Pawel Drzycimski
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
#include "simplelog/simplelog.hpp"

#include "Optional.hpp"

int main()
{

  // integer
  {
    Optional<int> empty;
    LOG << "Empty has_value  : " << (empty.has_value() ? "FAILED!" : "Ok");
    LOG << "Empty has_value  : " << (empty ? "FAILED!" : "Ok");
    LOG << "Empty value_or   : " << (empty.value_or(5) == 5 ? "Ok" : "FAILED!");
    LOG << "";
    Optional<int> integer(10);
    LOG << "Integer has_value: " << (integer.has_value() ? "Ok" : "FAILED!");
    LOG << "Integer has_value: " << (integer ? "Ok" : "FAILED!");
    LOG << "Integer value    : " << (integer.value() == 10 ? "Ok" : "FAILED!");
    LOG << "Integer value_or : " << (integer.value_or(5) == 10 ? "Ok" : "FAILED!");
    LOG << "";
    integer.reset(); 
    LOG << "Reset has_value  : " << (integer.has_value() ? "FAILED!" : "Ok");
    LOG << "Reset has_value  : " << (integer ? "FAILED!" : "Ok");
    LOG << "Reset value_or   : " << (integer.value_or(5) == 5 ? "Ok" : "FAILED!");
 
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
