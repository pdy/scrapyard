#include <napi.h>
#include <vector>

#include "MyObjects.cpp"
#include "async_workers.cpp"

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
