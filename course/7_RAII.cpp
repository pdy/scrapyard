#include "simplelog/simplelog.hpp"
#include <mutex>
#include <type_traits>

template<typename T, auto Deleter>
struct GenericDeleter
{
  void operator()(T *rc) const
  {
    LOG << "generic deleter";
    Deleter(rc);
  }
};

template<typename T, auto Func>
using legacy_ptr = std::unique_ptr<T, GenericDeleter<T, Func>>;


// Resource Acquisition Is Initialization

struct StackUnwind
{
  size_t count {0};

  ~StackUnwind()
  {
    LOG << "Unwind " << count;
  }
};

static void throw_exception()
{
  StackUnwind s1 { 5 };
  StackUnwind s2 { 6 };
  LOG << "Throwing exception String";
//  throw "String";
  StackUnwind s3 { 7 };
  StackUnwind s4 { 8 };
}

template<typename T>
class ScopeGuard
{
  T func_;
public:
  ScopeGuard(T func)
    : func_(std::move(func))
  {
    static_assert(std::is_invocable<T>::value);
  }

  ~ScopeGuard()
  {
    func_();
  }
};

template<typename T>
[[nodiscard]] ScopeGuard<T> makeScopeGuard(T callable)
{
  return ScopeGuard<T>(std::move(callable));
}


using File_uptr = std::unique_ptr<std::FILE, decltype(&std::fclose)>;

using File_uptr_2 = legacy_ptr<std::FILE, std::fclose>;

struct FileStream
{
  File_uptr_2 file_;

  
  void setFIlename();
  bool streamTo(std::ostream &ss);
};

int main()
{
  size_t counter = 5;
  LOG << "counter " << counter;
  {
    const auto decreaseCounter = makeScopeGuard([&counter]
    {
      --counter;
    });
  }
  LOG << "counter " << counter;


  std::FILE *file {nullptr};
  const auto fileGuard = makeScopeGuard([&] { if(file) std::fclose(file);});

  if(file)
    std::fclose(file);


  std::mutex mtx;
  {
    std::lock_guard<std::mutex> lock (mtx);
  }

  
//  StackUnwind s1 { 1 };
  /*StackUnwind s2 { 2 };
  StackUnwind s3 { 3 };
  StackUnwind s4 { 4 };
  */
 // try{
//    throw_exception();
  //  StackUnwind s3 { 3 };
  //  StackUnwind s4 { 4 };
 // }
//  catch(const char *str)
//  {
//    LOG << "Catched exception " << str;
//  }

  return 0;
}
