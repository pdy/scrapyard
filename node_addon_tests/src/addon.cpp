#include <napi.h>
#include <vector>
#include "MyObjects.h"

#include <iostream>

namespace {

void byte_array_delete(Napi::Env /*env*/, uint8_t *arr, const char *hint) 
{
  std::cout << "BYTE_ARRAY_DELETE " << hint << "\n";
  delete[] arr;
}

class DataProcessingAsyncWorker : public Napi::AsyncWorker
{
public:
  //
  DataProcessingAsyncWorker(Napi::Function &callback, std::string str, int idx)
    : AsyncWorker(callback), m_val{std::move(str)}, m_idx{idx}
  {}

  void Execute() override
  {

    if(m_idx < 0)
      Napi::AsyncWorker::SetError("Negative idx");
    else if(static_cast<size_t>(m_idx) >= m_val.size() - 1)
      Napi::AsyncWorker::SetError("Idx too high");
    else
      m_val = m_val.substr(static_cast<size_t>(m_idx));
  }

  void OnOK() override
  {
    Napi::HandleScope scope(Env());

    Callback().Call({
        Env().Null(),
        Napi::String::New(Env(), m_val)
    });
  }

private:
  std::string m_val;
  int m_idx;
};


class BufferReverseAsync : public Napi::AsyncWorker
{
public:
  BufferReverseAsync(Napi::Function &callback, std::unique_ptr<uint8_t[]> data, size_t size)
    : AsyncWorker(callback), m_inBuffer{std::move(data)}, m_size{size}
  {}

  void Execute() override
  {
    for(size_t beg = 0, end = m_size - 1; beg < end; ++beg, --end)
    {
      const auto tmp = m_inBuffer[beg];
      m_inBuffer[beg] = m_inBuffer[end];
      m_inBuffer[end] = tmp;
    }

  }

  void OnOK() override
  {
    Napi::HandleScope scope(Env()); 

    Callback().Call({
        Env().Null(),
        Napi::Buffer<uint8_t>::New(
            Env(),
            m_inBuffer.release(),
            m_size,
            byte_array_delete,
            "BYTE ARRAY REVERSED")
    });
  }

private:
  std::unique_ptr<uint8_t[]> m_inBuffer{nullptr};
  size_t m_size {0};
};

} // namespace

void ReverseByteBuffer(const Napi::CallbackInfo &info)
{
  auto byteBuffer = info[0].As<Napi::Buffer<uint8_t>>();
  Napi::Function callback = info[1].As<Napi::Function>(); 

  auto buffer = std::make_unique<uint8_t[]>(byteBuffer.ByteLength());
  for(size_t i = 0; i < byteBuffer.ByteLength(); ++i)
  {
    buffer[i] = byteBuffer[i];
  }

  auto *async = new BufferReverseAsync(callback, std::move(buffer), byteBuffer.ByteLength());
  async->Queue();
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
    exports.Set(Napi::String::New(env, "reverseByteBuffer"), Napi::Function::New(env, ReverseByteBuffer));

    Napi::String helloStrName = Napi::String::New(env, "HelloStr");
    exports.Set(helloStrName, HelloStr::GetClass(env));


    return exports;
};

NODE_API_MODULE(hello_world, init);
