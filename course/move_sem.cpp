#include "simplelog/simplelog.hpp"
#include <cstring>
#include <type_traits>

#if 0
class String
{
  std::string m_str;

  const char *m_objName{nullptr};
public:
  String(const std::string &str, const char *objName = "")
      : m_str{str}, m_objName{objName}
  {}

  String(const String &other)
  {
    LOG << "String [ " << m_objName << "] copy ctor";
    m_str = other.m_str;
  }

  size_t size() const { return m_str.size(); }
  const char *data() const { return m_str.data(); }
};

int main()
{
  String str1("Hello", "str1");
  LOG << str1.data();
  return 0;
}
#endif

// #if 0
class String
{
  char *m_str{nullptr};
  size_t m_size{0};

  const char *m_objName{nullptr};

public:
  String(const char *str, const char *objName)
    : m_objName{objName}
  {
    LOG << "String [" << m_objName << "] convert ctor";
    if(str)
    {
      m_size = strlen(str);
      m_str = new (std::nothrow) char[m_size];
      if(m_str)
        memcpy(m_str, str, m_size);
    }
  }

  ~String()
  {
    LOG << "String [" << m_objName << "] call destructor";
    if(m_str)
      delete[] m_str;
  }

  size_t size() const { return m_size; }
  const char *data() const { return m_str; }
};

int main()
{
  String str("Hello", "str");

//  LOG << str.data();

  return 0;
}

template<typename T>
void log_lval_rval_resolution(T &&val)
{
//  LOG << "I'am rvalue: " << std::is_rvalue_reference_v<T>;
  LOG << "I'am rvalue: " << std::is_same<T&&, decltype(val)>::value;
  LOG << "I'am lvalue: " << std::is_lvalue_reference_v<T>;
}
//#endif
