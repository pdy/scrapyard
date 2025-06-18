/*
*  MIT License
*  
*  Copyright (c) 2025 Pawel Drzycimski
*  
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*  
*  The above copyright notice and this permission notice shall be included in all
*  copies or substantial portions of the Software.
*  
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*  SOFTWARE.
*
*/

#include "simplelog/simplelog.hpp"

#include <thread>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>

#define KB (static_cast<size_t>(1024))
#define MB (static_cast<size_t>(1024) * KB)
#define GB (static_cast<size_t>(1024) * MB)

#ifdef _WIN32
    #define NEWLINE "\r\n"
    static constexpr size_t NEWLINE_LEN = 2;
#else
    #define NEWLINE "\n"
    static constexpr size_t NEWLINE_LEN = 1;
#endif


namespace fs = std::filesystem;

struct AppArgs
{
  std::string_view filename;
  std::string_view searchExtension;
};

static void usage()
{
  LOG << "log_merger -f <file name> -e <extension>";
  LOG << "  -f <file name> - file name to output merged logs";
  LOG << "  -e <extension> - case sensitive with dot ex. .txt, .log";
}

struct FileGuardDeleter
{
  void operator()(FILE *file) const
  {
    fclose(file);
  }
};

using file_guard = std::unique_ptr<FILE, FileGuardDeleter>;

struct FnamesMemory
{
  std::unique_ptr<char[]> memory {nullptr};
  size_t count {0};

  const size_t maxCount {0};
  const size_t regionSize {0};

  explicit operator bool() const { return memory != nullptr; }

  char *data() const { return memory.get(); }
  bool empty() const { return count == 0; };

  bool append(std::string_view str)
  {
    if(count == maxCount)
      return false;

    char *ptr = data() + regionSize * count;
    std::memset(ptr, 0x00, regionSize);
    std::memcpy(ptr, str.data(), str.size());
    ++count;

    return true;
  }

  std::string_view get(size_t idx) const
  {
    assert(idx < count && "Buffer overflow");
    return data() + idx * regionSize;
  }

  std::string_view last() const
  {
    return data() + (count - 1) * regionSize;
  }

  void pop_last()
  {
    const size_t idx = (count - 1) * regionSize;
    std::memset(data() + idx, 0x00, regionSize);
    --count;
  }
};

static FnamesMemory initFnamesMem(size_t count, size_t regionSize)
{
  char *data = new(std::nothrow) char[count * regionSize];
  if(!data)
    return FnamesMemory{};

  return FnamesMemory{
    .memory{data},
    .count = 0,
    .maxCount = count,
    .regionSize = regionSize
  };
}

static std::unique_ptr<uint8_t[]> allocBuffer(size_t size)
{
  return std::unique_ptr<uint8_t[]> {new(std::nothrow) uint8_t[size] };
}

#if 0
static size_t get_size(FILE *file)
{
  fseek(file, 0, SEEK_END);
  const long ret = ftell(file);
  fseek(file, 0, SEEK_SET); 

  return ret;
}
#endif

int main(int argc, char *argv[])
{
  static constexpr size_t FILE_COUNT_LIMIT = 1'048'576;
  static constexpr size_t FNAME_MAX_SIZE = 450;
  static constexpr size_t READ_BUFF_SIZE = 2 * GB + 10; // +10 just in case
//  static constexpr size_t FNAME_POOL_SIZE = FNAME_MAX_SIZE * FILE_COUNT_LIMIT;

  if(argc != 5)
  {
    LOG << "Missing input parameters!";
    usage();
    return 0;
  }

  const auto [filename, extension] = [argc, argv] {
    AppArgs ret;
    for(size_t i = 1; i < static_cast<size_t>(argc); ++i)
    {
      if(std::strcmp("-f", argv[i]) == 0)
        ret.filename = argv[i + 1];
      else if(std::strcmp("-e", argv[i]) == 0)
        ret.searchExtension = argv[i + 1];
    }
    return ret;
  }();

  if(filename.empty())
  {
    LOG << "Missing -f parameter!";
    usage();
    return 0;
  }
  if(extension.empty())
  {
    LOG << "Missing -e parameter!";
    usage();
    return 0;
  }
  if(!extension.starts_with('.'))
  {
    LOG << "Extension need to start wth a dot!";
    usage();
    return 0;
  }

//  LOG << "file limit " << FILE_COUNT_LIMIT;
//  LOG << "filename " << filename << " extension " << extension;

  FnamesMemory fnamesArray = initFnamesMem(FILE_COUNT_LIMIT, FNAME_MAX_SIZE);
  if(!fnamesArray)
  {
    LOG << "Cant initizalize memory to hold file names";
    return 1;
  }

  auto readBuffer = allocBuffer(READ_BUFF_SIZE);
  if(!readBuffer)
  {
    LOG << "Cant initialize memory for I/O";
    return 1;
  }

  FILE *mergedLog = fopen(std::string{filename}.c_str(), "wb");
  if(!mergedLog)
  {
    LOG << "Couldn't open merged.log for writing";
    return 1;
  }
  
  file_guard writeFileGuard{mergedLog};

  std::mutex fnamesMutex;
  std::atomic_bool finishedPathTraversal = false;
  std::condition_variable fnameAccess;
  std::jthread writeThread([&]
  {
    LOG << "Enterig write thred";
    while(!finishedPathTraversal)
    {
      std::unique_lock lock(fnamesMutex);
      fnameAccess.wait(lock, [&] { return !fnamesArray.empty() || finishedPathTraversal; });
     
      if(finishedPathTraversal)
        break;

      LOG << " Locked mutex";
      const std::string fname{fnamesArray.last()};
      fnamesArray.pop_last();

      lock.unlock();
      fnameAccess.notify_one();

      if(fname.empty())
        continue;

      LOG << "Trying to read " << fname;
      FILE *file{ fopen(fname.c_str(), "rb") };
      if(file)
      {
        LOG << " File open ok";
        file_guard fguard{file};
        const auto bytesRead = fread(readBuffer.get(), 1, READ_BUFF_SIZE, file);
        if(const int error = ferror(file); error != 0)
        {
          LOG << "Error [" << error << "] while reading " << fname;
        }
        else if(bytesRead == 0)
        {
          LOG << "Empty file " << fname;
        }
        else
        {
          LOG << " File read ok";
          LOG << " Writing";
          const size_t bytesWritten = fwrite(readBuffer.get(), 1, bytesRead, mergedLog);
          if(bytesWritten != bytesRead)
          {
            LOG << "Error [" << ferror(mergedLog) << "] while writing. ABORTING!";
            return;
          }

          LOG << " File write ok";
          fwrite(NEWLINE, 1, NEWLINE_LEN, mergedLog);
        }
      }
      else
      {
        LOG << "Couldn't read " << fname;
      }
    }

    // finishedPathTraversal == true
    // we have fnameArray for ourselves so no more copies and locking
    while(!fnamesArray.empty())
    {
      LOG << "!!!! IMPLEMENT ME !!!!";

      const std::string fname {fnamesArray.last()};  
      fnamesArray.pop_last();

      LOG << "Trying to read " << fname;
      FILE *file{ fopen(fname.c_str(), "rb") };
      if(file)
      {
        LOG << " File open ok";
        file_guard fguard{file};
        const auto bytesRead = fread(readBuffer.get(), 1, READ_BUFF_SIZE, file);
        if(const int error = ferror(file); error != 0)
        {
          LOG << "Error [" << error << "] while reading " << fname;
        }
        else if(bytesRead == 0)
        {
          LOG << "Empty file " << fname;
        }
        else
        {
          LOG << " File read ok";
          LOG << " Writing";
          const size_t bytesWritten = fwrite(readBuffer.get(), 1, bytesRead, mergedLog);
          if(bytesWritten != bytesRead)
          {
            LOG << "Error [" << ferror(mergedLog) << "] while writing. ABORTING!";
            return;
          }

          LOG << " File write ok";
          fwrite(NEWLINE, 1, NEWLINE_LEN, mergedLog);
        }
      }
      else
      {
        LOG << "Couldn't read " << fname;
      }
    }

    LOG << "Exiting read/write thread";
  });

  const fs::path fsExtension{extension};
  const fs::path fsFname{filename};
  for(auto itEntry = fs::recursive_directory_iterator("./");
      itEntry != fs::recursive_directory_iterator();
      ++itEntry)
  {
    const auto &path = itEntry->path();
    if(path.filename() == fsFname || fs::is_directory(path) || path.extension() != fsExtension)
    {
      LOG << "Ommiting " << path.filename().string();
      continue;
    }

    {
      std::lock_guard lock(fnamesMutex);
      fnamesArray.append(path.string());
//    LOG << "Added to name pool [" << fnamesArray.last() << "]";
    }
    fnameAccess.notify_one();
  }
  finishedPathTraversal = true;
  fnameAccess.notify_all();

#if 0
  for(size_t i = 0; i < fnamesArray.count; ++i)
  {
    const auto val = fnamesArray.get(i);
    LOG << "Added to name pool [" << val << "] len [" << val.size() << ']';
  }
#endif

  return 0;
}
