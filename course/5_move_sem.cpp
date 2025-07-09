#include "simplelog/simplelog.hpp"
#include <cstring>

static std::string&& myMove( std::string &t) noexcept
{
  return static_cast<std::string&&>(t);
}

static std::string stringOperation(std::string str)
{
  std::string ret = std::move(str);
  return ret;
}

struct StringHolder
{
  std::string str;
};

template<typename T>
void log_lval_rval_resolution(T &&val)
{
  LOG << "  I'am rvalue ref: " << std::is_rvalue_reference_v<decltype(val)>;
  LOG << "  I'am lvalue ref: " << std::is_lvalue_reference_v<decltype(val)>;

  std::forward<T>(val);
}

int main()
{
  // perfect forwarding

  std::vector<std::string> vecOfStrings;
  stringOperation(std::move( vecOfStrings[0]));

  std::string str;
  LOG << "Log str value";
  log_lval_rval_resolution(str);
  LOG << "Log int std::string{}";
  log_lval_rval_resolution(std::string{});

#if 0
  std::string str_1 = "some value";

  LOG << "str_1 value [" << str_1 << "]";
  LOG << "str_1 size " << str_1.size();

  LOG << "Moving str_1";
  std::string str_2 = stringOperation(str_1);

  LOG << "str_2 value [" << str_2 << "]";
  LOG << "str_2 size " << str_2.size();

  LOG << "str_1 value [" << str_1 << "]";
  LOG << "str_1 size " << str_1.size();
#endif

  return 0;
}
