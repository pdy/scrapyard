#include "simplelog/simplelog.h"

#include <memory>
#include <type_traits>

#include <optional>

struct Type 
{
  int placeholder{0};

  ~Type() {}
};

int main()
{
  
  Type *type = new Type{3};

  std::optional<Type> opt;

  std::cout << "is_trivially_destructible: " << std::is_trivially_destructible<Type>::value << "\n";
  std::cout << "addressof [" << std::addressof(type) << "] val [" << type->placeholder << "]\n";

  type->~Type();
  std::cout << "addressof [" << std::addressof(type) << "] val [" << type->placeholder << "]\n";
  
  std::construct_at(std::addressof(*type) , 5);

  std::cout << "addressof [" << std::addressof(type) << "] val [" << type->placeholder << "]\n";
  
  return 0;
}
