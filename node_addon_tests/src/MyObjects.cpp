#include "MyObjects.h"
#include "napi.h"

HelloStr::HelloStr(const Napi::CallbackInfo &info)
  : ObjectWrap(info)
{
  Napi::Env env = info.Env();

  if(info.Length() != 1)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return;
  }

  if(!info[0].IsString())
  {
    Napi::TypeError::New(env, "Argument need to be string").ThrowAsJavaScriptException();
    return;
  }

  m_str = info[0].As<Napi::String>();
}

Napi::Value HelloStr::Substr(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  
  if(info.Length() != 1)
  {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }
  
  if(!info[0].IsNumber())
  {
    Napi::TypeError::New(env, "Argument need to be number").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }

  const int idx = info[0].As<Napi::Number>();
  if(idx < 0)
  {
    Napi::TypeError::New(env, "Argument idx needs to be zero or higher").ThrowAsJavaScriptException();
    return Napi::String::New(env, "");
  }


  m_str = m_str.substr(static_cast<size_t>(idx));
  
  return Napi::String::New(env, m_str);
}


Napi::Function HelloStr::GetClass(Napi::Env env)
{
  return DefineClass(env, "HelloStr", {
      InstanceMethod("substr", &HelloStr::Substr),
  });
}
