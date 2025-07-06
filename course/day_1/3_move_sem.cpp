#include "simplelog/simplelog.hpp"
#include <cstring>
//#include <type_traits>

#if 0
class String
{
  std::string m_str;

  const char *m_objName{nullptr};
public:
  String(const std::string &str, const char *objName = "")
      : m_str{str}, m_objName{objName}
  {}

  String(String &&other)
    : m_str{std::move(other.m_str)}, m_objName{other.m_objName}
  {
    LOG << "String " << m_objName << " move ctor";
  }

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


  String str2(std::move(str1));

  LOG << "String moved from " << str1.size();
  LOG << "String str2 size " << str2.size();



  return 0;
}
#endif

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

  String& operator=(String &&other) noexcept
  {
    if(*this == other)
      return *this;

    if(m_str)
    {
      delete[] m_str;
      m_size = 0;
    }



    return *this;
  }

};


static String construct(const char *str)
{
  String ret(str, "ret");
//  ret.

  return ret;

}

int main()
{
  /*
  String str("Hello", "str");
  LOG << str.data();
  

  String str2("hello2", "str2");
  str2 = std::move(str);
*/
  
  String str = construct("Hello");








#if 0
  LOG << "L-VALUE/R-VALUE check [str]";
  log_lval_rval_resolution(str);
  LOG << "L-VALUE/R-VALUE check [5]";
  log_lval_rval_resolution(5);
#endif

  return 0;
}

#if 0
template<typename T>
void log_lval_rval_resolution(T &&val)
{
  LOG << "  I'am rvalue ref: " << std::is_rvalue_reference_v<decltype(val)>;
  LOG << "  I'am lvalue ref: " << std::is_lvalue_reference_v<decltype(val)>;
}
#endif
