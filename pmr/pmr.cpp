/*
*  MIT License
*  
*  Copyright (c) 2026 Pawel Drzycimski
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

#include <cstdlib>
#include <memory_resource>
#include <new>

#define KB (1024)
#define MB (KB * 1024)


static size_t g_allocMemoryCount = 0, g_allocateCount = 0;


class NonThrowMemoryUpstream : public std::pmr::memory_resource
{
  using Base = std::pmr::memory_resource;

public:
  using Base::Base;
//  using Base::allocate;
//  using Base::deallocate;


protected:
  void* do_allocate(std::size_t bytes, std::size_t align) override
  {
    void *ptr = nullptr;//std::aligned_alloc(align, bytes);
    if(!ptr)
      throw std::bad_alloc();

    ++g_allocMemoryCount;
    return ptr;
  }

  void do_deallocate(void *p, std::size_t /*bytes*/, std::size_t /*align*/) override
  {
    std::free(p);
  }

  bool do_is_equal(const std::pmr::memory_resource &other) const noexcept override 
  {
    std::cout << "do_is_equal\n";
    return dynamic_cast<const NonThrowMemoryUpstream*>(&other) != nullptr;
  }
};

class AllocStat : public std::pmr::monotonic_buffer_resource
{
  using Base = std::pmr::monotonic_buffer_resource;

public:
  using Base::Base;
  using Base::allocate;
  using Base::deallocate;

protected:
  void* do_allocate(std::size_t count, std::size_t alignment) override
  {
    ++g_allocateCount;
    return Base::do_allocate(alignment, count);
  }

};


static void run()
{
  NonThrowMemoryUpstream additionalMemory;
  AllocStat arena(1 * MB, &additionalMemory);

  //std::pmr::monotonic_buffer_resource arena(1 * KB, &additionalMemory);
  const std::string_view text = "SomeString";
  std::pmr::string str(&arena); 
  for(size_t i = 0; i < 99; ++i)
    str.append(text);

  
  std::pmr::string str2(&arena);
  str2 = str;

  
  //std::cout << '[' << str.size() << "] " << str << '\n';

  std::cout << "allocate \t[" << g_allocateCount << "]\nalloc memory \t[" << g_allocMemoryCount << "]\n";
}

int main(int /*argc*/, char**)
{
  LOG << "Hello PMR!";
  
  try {
    run();
  }catch(std::bad_alloc &e) {
    std::cout << "Bad alloc thrown: " << e.what() << '\n';
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
