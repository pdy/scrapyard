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

#include <cmath>
#include <cmdline.h> 
#include <cstddef>
#include <cstdlib>
#include <limits>
#include "simplelog/simplelog.hpp"

#include "Array.hpp"

template<typename T>
void test(const ArrayView<T> &view)
{
  LOG << "size: " << view.size();
  LOG << "empty: " << view.empty();
  std::cout << "values: ";
  for(const auto val : view)
    std::cout << val << " ";
  std::cout << '\n';
  
  std::cout << "values: ";
  for(size_t i = 0; i < view.size(); ++i)
    std::cout << view[i] << " ";
  std::cout << '\n';
}

struct SomeType
{
  short content{0};
  float fcontent{.0};
};

inline std::ostream& operator<<(std::ostream &oss, const SomeType &st)
{
  return oss << "short [" << st.content << "] float [" << st.fcontent << "]";
}

template<typename T, size_t SIZE, typename CastType = int>
inline void printArray(T (&arr)[SIZE])
{
  std::cout << "buff value [";
  for(size_t i = 0; i < SIZE; ++i)
  {
    std::cout << std::dec << static_cast<CastType>(arr[i]);
    if(i != SIZE - 1)
      std::cout << " ";
  }
  std::cout << ']';
}

inline bool isPowerofTwo(size_t n)
{
  return (n & (n - 1)) == 0;
#if 0
  if (n <= 0)
    return false;
  
  // Calculate log base 2 of n
  const int logValue = (int)std::log2(n);
  
  // Check if log2(n) is an integer
  // and 2^(logn) = n
  return pow(2, logValue) == n;
#endif
}

inline bool isDivisibleBy(uintptr_t n, size_t by)
{
  return n % by == 0;
}

int main()
{
  /*
  LOG << "Hello Array!";

  int integers[10] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  
  ArrayView<int> intView(integers);
  test(intView);

  const SomeType structs[4] { {0, 0.1f}, {1, 0.2f}, {2, 0.3f}, {3, 0.4f}};
  ArrayView<const SomeType> podView(structs);
  test(podView);
*/


  static constexpr size_t ALIGN = alignof(SomeType);
  LOG << "ALIGN power of two: " << isPowerofTwo(ALIGN);
  static constexpr size_t SOME_TYPE_SIZE = sizeof(SomeType);
  /*alignas(SomeType)*/ uint8_t someTypeBuffer[SOME_TYPE_SIZE + ALIGN - 1] = {0};
//  char *aligned = (char*)((intptr_t)buf + (alignment - 1) & ~intptr_t(alignment - 1));
  //uint8_t *ptr = (uint8_t*)((intptr_t)someTypeBuffer + (ALIGN - 1) & ~intptr_t(ALIGN - 1));
  uint8_t *ptr = reinterpret_cast<uint8_t*>(
      (reinterpret_cast<intptr_t>(someTypeBuffer) + (ALIGN - 1)) & ~(ALIGN - 1));

//  LOG << "someTypeBuffer pow2 [" << isPowerofTwo((uintptr_t)someTypeBuffer) << "] ptr pow2 [" << isPowerofTwo((uintptr_t)ptr) << ']';
  LOG << "someTypeBuffer div" << ALIGN << " [" << isDivisibleBy((uintptr_t)someTypeBuffer, ALIGN) << "] ptr div" << ALIGN  << " [" << isDivisibleBy((uintptr_t)ptr, ALIGN) << ']';

  //uint8_t *someTypeBuffer = new uint8_t(SOME_TYPE_SIZE);
//  std::cout << "ptr: " << std::dec << (void*)ptr << '\n';

  LOG 
    << "buff size: " << std::size(someTypeBuffer) 
    << " SomeType size: " << sizeof(SomeType)
    << " start address: [" << someTypeBuffer 
    << "] align address: [" << ptr 
    << "] end address [" << (someTypeBuffer + std::size(someTypeBuffer)) 
    << "] align diff [" << (someTypeBuffer + std::size(someTypeBuffer)) - ptr << ']';
  
  printArray(someTypeBuffer);
  LOG << "";

  SomeType *someTypeNew = ::new(ptr) SomeType;
  someTypeNew->content = 5;//std::numeric_limits<short>::max();
  someTypeNew->fcontent = std::numeric_limits<float>::max();
  printArray(someTypeBuffer);
  LOG << "";
  LOG << "content: " << someTypeNew->content;
  LOG << "fcontent: " << someTypeNew->fcontent;

  LOG << "SomeType align: " << alignof(SomeType);
//  LOG << "max align: " << alignof(std::max_align_t);


  LOG << "is 2 power of two: " << isPowerofTwo(2);
  LOG << "is 4 power of two: " << isPowerofTwo(4);

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
