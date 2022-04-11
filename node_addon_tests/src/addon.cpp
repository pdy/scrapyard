#include <napi.h>
#include "MyObjects.h"

namespace {

class DataProcessingAsyncWorker : public Napi::AsyncWorker
{
public:
  //
  DataProcessingAsyncWorker(Napi::Function &callback, std::string str, int idx)
    : AsyncWorker(callback), m_val{std::move(str)}, m_idx{idx}
  {}

  void Execute()
  {
    if(m_idx < 0)
      Napi::AsyncWorker::SetError("Negative idx");
    else if(static_cast<size_t>(m_idx) >= m_val.size() - 1)
      Napi::AsyncWorker::SetError("Idx too high");
    else
      m_val = m_val.substr(static_cast<size_t>(m_idx));
  }

  void OnOK()
  {
    Callback().Call({Env().Null(), Napi::String::New(Env(), m_val)});
  }

private:
  std::string m_val;
  int m_idx;
};

class BufferTestWorker : public Napi::AsyncWorker
{
public:
  

private:
  Napi::Buffer<uint8_t> m_byteBuffer;
};

} // namespace

void ReverseByteBuffer(const Napi::CallbackInfo &info)
{
  const auto env = info.Env();
  Napi::HandleScope scope(env);

  auto byteBuffer = info[0].As<Napi::Buffer<uint8_t>>();
  Napi::Function callback = info[1].As<Napi::Function>(); 

  
  auto retBuffer = Napi::Buffer<uint8_t>::New(env, byteBuffer.ByteLength());
  
  size_t j = 0;
  for(auto i = byteBuffer.Length(); i > 0; --i,++j)
  {
    retBuffer[j] = byteBuffer[i - 1];
  }
}

Napi::String SayHi(const Napi::CallbackInfo& info)
{
  Napi::Env env = info.Env();


  return Napi::String::New(env, "Hi!");
}

void helloSubstr(const Napi::CallbackInfo &info)
{
  std::string str = info[0].As<Napi::String>();
  const int idx = info[1].As<Napi::Number>();
  Napi::Function callback = info[2].As<Napi::Function>();

  auto *async = new DataProcessingAsyncWorker(callback, std::move(str), idx);
  async->Queue();  
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "sayHi"), Napi::Function::New(env, SayHi));
    exports.Set(Napi::String::New(env, "helloSubstr"), Napi::Function::New(env, helloSubstr));

    Napi::String helloStrName = Napi::String::New(env, "HelloStr");
    exports.Set(helloStrName, HelloStr::GetClass(env));


    return exports;
};

NODE_API_MODULE(hello_world, init);
