#include <napi.h>

namespace {

class DataProcessingAsyncWorker : public Napi::AsyncWorker
{
public:
  //
  DataProcessingAsyncWorker(Napi::Function &callback, std::string str)
    : AsyncWorker(callback), m_val{std::move(str)}
  {}

  void Execute()
  {
    if(m_val.empty())
      SetError("Empty string");

  }

  void OnOK()
  {
    Callback().Call({Env().Null(), Napi::String::New(Env(), m_val)});
  }

private:
  std::string m_val;
};

} // namespace


Napi::String SayHi(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();


  return Napi::String::New(env, "Hi!");
}

void helloSubstr(const Napi::CallbackInfo &info)
{
  const std::string str = info[0].As<Napi::String>();
  const int idx = info[1].As<Napi::Number>();
  Napi::Function callback = info[2].As<Napi::Function>();

  std::string val;
  if(idx >= 0)
    val = str.substr(static_cast<size_t>(idx));

  auto *async = new DataProcessingAsyncWorker(callback, std::move(val));
  async->Queue();  
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "sayHi"), Napi::Function::New(env, SayHi));
    exports.Set(Napi::String::New(env, "helloSubstr"), Napi::Function::New(env, helloSubstr));

    return exports;
};

NODE_API_MODULE(hello_world, init);
