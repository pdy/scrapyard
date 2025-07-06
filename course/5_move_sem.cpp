#include "simplelog/simplelog.hpp"
/*
    std::string&&
    move(std::string && t) 
    { return static_cast<std::string&&>(t); }
 *
 */

static std::string&& moveStr(std::string &str)
{
  return static_cast<std::string&&>(str);
}

int main()
{
  std::string str_1 = "some value";
  LOG << "str_1 size " << str_1.size();

  LOG << "Moving str_1";
//  auto str_2 = std::move(str_1);
  auto str_2 = moveStr(str_1);
  LOG << "str_1 size " << str_1.size();
  LOG << "str_2 size " << str_2.size();
  LOG << "str_2 value " << str_2;

  return 0;
}
