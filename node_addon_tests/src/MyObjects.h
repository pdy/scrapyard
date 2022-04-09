#ifndef PDY_MY_OBJECTS_H_
#define PDY_MY_OBJECTS_H_

#include <napi.h>

#include <string>

class HelloStr : public Napi::ObjectWrap<HelloStr>
{
public:
  HelloStr(const Napi::CallbackInfo&);
  Napi::Value Substr(const Napi::CallbackInfo&);

  static Napi::Function GetClass(Napi::Env);

private:
  std::string m_str;
};

#endif
