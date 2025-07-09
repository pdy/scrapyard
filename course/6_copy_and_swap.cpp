#include "simplelog/simplelog.hpp"
#include <cstring>
#include <utility>
#include <vector>

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
      m_size = strlen(str) + 1;
      m_str = new (std::nothrow) char[m_size];
      if(m_str)
        memcpy(m_str, str, m_size);
    }
  }

  String(String &&other) noexcept
  {
    m_str = other.m_str;
    m_size = other.m_size;

    other.m_str = nullptr;
    other.m_size = 0; 

    m_objName = other.m_objName;
    other.m_objName = "Moved From";

    LOG << "String " << m_objName << " move ctor";
  }

  ~String()
  {
    LOG << "String [" << m_objName << "] call destructor";
    if(m_str)
      delete[] m_str;
  }

  size_t size() const { return m_size; }
  const char *data() const { return m_str; }

  bool operator ==(const String &other) const
  {
    return m_size == other.m_size && m_str == other.m_str;
  }

  String& operator=(const String &other)
  {
    if(*this == other)
      return *this;


    if(m_str)
    {
      delete[] m_str;
      m_size = 0;
    }

    m_size = other.m_size;
    m_str = new char[m_size];
    std::memcpy(m_str, other.m_str, m_size); 

    return *this;
  }

  String& operator=(String &&other) 
  {
    if(*this == other)
      return *this;

    if(m_str)
    {
      delete[] m_str;
      m_size = 0;
    }

    m_str = other.m_str;
    m_size = other.m_size;

    other.m_str = nullptr;
    other.m_size = 0;

    return *this;
  }

};


 // copy and swap idiom
 
class String_2
{
  std::string m_str{nullptr};
  size_t m_size {0};

public:
  
  String_2() = default;
  String_2(const String_2 &other)
  {
    m_size = other.m_size;
    m_str = other.m_str;
//    std::memcpy(m_str, other.m_str, m_size);
  }

  String_2& operator=(String_2 other) noexcept
  {
    LOG << "Copy and Swap for the win";
    swap(*this, other);
    return *this;
  }

 friend void swap(String_2 &lhs, String_2 &rhs) noexcept
 {
   using std::swap;

   swap(lhs.m_size, rhs.m_size);
   swap(lhs.m_str, rhs.m_str);
 } 

};

int main()
{
  String_2 str;
  
  String_2 str_2 = str;
  String_2 str_3 = std::move(str);

  return 0;
}













