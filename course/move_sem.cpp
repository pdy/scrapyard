#include "simplelog/simplelog.hpp"
#include <cstring>
#include <new>

class String
{
  char *m_str{nullptr};
  size_t m_size{0};

public:
  String(const char *str)
  {
    LOG << "String: call convert ctor";
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
    LOG << "String: call destructor";
    if(m_str)
      delete[] m_str;
  }

  size_t size() const { return m_size; }
  char *data() const { return m_str; }
};

int main()
{
  String str("Hello move semantics");

  LOG << str.data();

  return 0;
}
