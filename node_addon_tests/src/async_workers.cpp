#include "napi.h"
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
