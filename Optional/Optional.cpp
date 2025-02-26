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
#include <type_traits>
#include <utility>
#include "simplelog/simplelog.hpp"

#include "Optional.hpp"

template<typename T>
constexpr bool size_check()
{
  return sizeof(Optional<T>) == sizeof(T) + std::alignment_of<T>::value;
}

int main()
{
    
  // properties testing
  {
    {
      using OptInt = Optional<int>;
      const bool swapNoexcept = noexcept(swap(std::declval<OptInt&>(), std::declval<OptInt&>()));
      const bool noexceptTraitCheck = internal::is_noexcept_swappable<OptInt>::value;
      LOG << "swap noexcept      : " << (swapNoexcept ? "Ok" : "FAILED!");
      LOG << "trait noexcept     : " << (noexceptTraitCheck ? "Ok" : "FAILED!");
      LOG << "sizeof optint      : " << (size_check<int>() ? "Ok" : "FAILED!");
    }
    {
      struct NonThrowMoveCtor { int dummy; NonThrowMoveCtor(NonThrowMoveCtor &&o) noexcept : dummy{std::move(o.dummy)} {} };
      using OptStruct = Optional<NonThrowMoveCtor>;
      
      const bool swapNoexcept = noexcept(swap(std::declval<OptStruct&>(), std::declval<OptStruct&>()));
      const bool noexceptTraitCheck = internal::is_noexcept_swappable<OptStruct>::value;
      LOG << "swap noexcept      : " << (swapNoexcept ? "Ok" : "FAILED!");
      LOG << "trait noexcept     : " << (noexceptTraitCheck ? "Ok" : "FAILED!");
      LOG << "sizeof             : " << (size_check<NonThrowMoveCtor>() ? "Ok" : "FAILED!");
    }
    {
      struct ThrowingMoveCtor { int dummy; ThrowingMoveCtor(ThrowingMoveCtor &&o) noexcept(false) : dummy{std::move(o.dummy)} {}};
      using OptThrowStruct = Optional<ThrowingMoveCtor>;
      
      const bool swapStructNoexcept = noexcept(swap(std::declval<OptThrowStruct&>(), std::declval<OptThrowStruct&>()));
      const bool noexceptStructTraitCheck = internal::is_noexcept_swappable<OptThrowStruct>::value;
      LOG << "swap noexcept      : " << (swapStructNoexcept ? "FAILED!" : "Ok");
      LOG << "trait noexcept     : " << (noexceptStructTraitCheck ? "FAILED!" : "Ok");
      LOG << "sizeof             : " << (size_check<ThrowingMoveCtor>() ? "Ok" : "FAILED!");
    }
  }

  LOG << "";

  // integer
  {
    Optional<int> empty;
    LOG << "Empty has_value    : " << (empty.has_value() ? "FAILED!" : "Ok");
    LOG << "Empty has_value    : " << (empty ? "FAILED!" : "Ok");
    LOG << "Empty value_or     : " << (empty.value_or(5) == 5 ? "Ok" : "FAILED!");
    LOG << "";
    Optional<int> integer(10);
    LOG << "Integer has_value  : " << (integer.has_value() ? "Ok" : "FAILED!");
    LOG << "Integer has_value  : " << (integer ? "Ok" : "FAILED!");
    LOG << "Integer value      : " << (integer.value() == 10 ? "Ok" : "FAILED!");
    LOG << "Integer value_or   : " << (integer.value_or(5) == 10 ? "Ok" : "FAILED!");
    LOG << "";
    Optional<int> assign = integer;
    LOG << "Assign has_value   : " << (assign.has_value() ? "Ok" : "FAILED!");
    LOG << "Assign has_value   : " << (assign ? "Ok" : "FAILED!");
    LOG << "Assign value       : " << (assign.value() == 10 ? "Ok" : "FAILED!");
    LOG << "Assign value_or    : " << (assign.value_or(5) == 10 ? "Ok" : "FAILED!");
    LOG << "Integer has_value  : " << (integer.has_value() ? "Ok" : "FAILED!");
    LOG << "Integer has_value  : " << (integer ? "Ok" : "FAILED!");
    LOG << "Integer value      : " << (integer.value() == 10 ? "Ok" : "FAILED!");
    LOG << "Integer value_or   : " << (integer.value_or(5) == 10 ? "Ok" : "FAILED!");
    LOG << "";
    integer.reset(); 
    LOG << "Reset has_value    : " << (integer.has_value() ? "FAILED!" : "Ok");
    LOG << "Reset has_value    : " << (integer ? "FAILED!" : "Ok");
    LOG << "Reset value_or     : " << (integer.value_or(5) == 5 ? "Ok" : "FAILED!");
    LOG << "";
    Optional<int> valAssign;
    valAssign = 10;
    LOG << "ValAssign has_value: " << (valAssign.has_value() ? "Ok" : "FAILED!");
    LOG << "ValAssign has_value: " << (valAssign ? "Ok" : "FAILED!");
    LOG << "ValAssign value    : " << (valAssign.value() == 10 ? "Ok" : "FAILED!");
    LOG << "ValAssign value_or : " << (valAssign.value_or(5) == 10 ? "Ok" : "FAILED!");
    LOG << "";
    Optional<int> inlAssign = 10;
    LOG << "InlAssign has_value: " << (inlAssign.has_value() ? "Ok" : "FAILED!");
    LOG << "InlAssign has_value: " << (inlAssign ? "Ok" : "FAILED!");
    LOG << "InlAssign value    : " << (inlAssign.value() == 10 ? "Ok" : "FAILED!");
    LOG << "InlAssign value_or : " << (inlAssign.value_or(5) == 10 ? "Ok" : "FAILED!");
    LOG << "";
  }  

  // simple struct
  {
    struct SimpleStruct
    {
      int integer{0};
      char chr{0};
      double f64{0};
    };
    
    Optional<SimpleStruct> simpleStruct = SimpleStruct { 10, 'n', 11.11 };
    LOG << "SimpleStruct sizeof     : " << (size_check<SimpleStruct>() ? "Ok" : "FAILED!");
    LOG << "SimpleStruct has_value  : " << (simpleStruct.has_value() ? "Ok" : "FAILED!");
    LOG << "SimpleStruct has_value  : " << (simpleStruct ? "Ok" : "FAILED!");
    LOG << "SimpleStruct value      : " << (simpleStruct.value().integer == 10 ? "Ok" : "FAILED!");
    LOG << "SimpleStruct value_or   : " << (simpleStruct.value_or(SimpleStruct{5, 0, 0}).integer == 10 ? "Ok" : "FAILED!");
    LOG << "";
    simpleStruct.reset();
    LOG << "SimpleStruct reset as_value  : " << (simpleStruct.has_value() ? "FAILED!" : "Ok");
    LOG << "SimpleStruct reset as_value  : " << (simpleStruct ? "FAILED!" : "Ok");
    LOG << "SimpleStruct reset value_or  : " << (simpleStruct.value_or(SimpleStruct{5, 0, 0}).integer == 5 ? "Ok" : "FAILED!");
    LOG << "";
    simpleStruct = SimpleStruct { 5, 'm', 22.22 };
    LOG << "AssignStruct has_value  : " << (simpleStruct.has_value() ? "Ok" : "FAILED!");
    LOG << "AssignStruct has_value  : " << (simpleStruct ? "Ok" : "FAILED!");
    LOG << "AssignStruct value      : " << (simpleStruct.value().integer == 5 ? "Ok" : "FAILED!");
    LOG << "AssignStruct value_or   : " << (simpleStruct.value_or(SimpleStruct{11, 0, 0}).integer == 5 ? "Ok" : "FAILED!");
    LOG << "";
 
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
