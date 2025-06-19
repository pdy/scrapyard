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

#include <optional>
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
//    static constexpr size_t NEWLINE_LEN = 1;
#endif


namespace fs = std::filesystem;

struct AppArgs
{
  std::string_view filename;
  std::string_view searchExtension;
};

static void usage()
{
  LOG << "";
  LOG << "!!!!! THIS IS UNFINISHED SLOW PROTOTYPE !!!!!";
  LOG << "!!!!!         Use log_merger_2           !!!!!";
  LOG << "";

  LOG << "log_merger -f <file name> -e <extension>";
  LOG << "  -f <file name> - file name to output merged logs";
  LOG << "  -e <extension> - case sensitive with dot ex. .txt, .log";
}

struct FileGuardDeleter
{
  void operator()(std::FILE *file) const
  {
    std::fclose(file);
  }
};

using FileGuard = std::unique_ptr<std::FILE, FileGuardDeleter>;

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

  /*
   * Requires valid idx
   */
  std::string_view get(size_t idx) const
  {
    assert(idx < count && "Buffer overflow");
    return data() + idx * regionSize;
  }

  /*
   * Requirest non empty
   */
  std::string_view last() const
  {
    return data() + (count - 1) * regionSize;
  }

  /*
   * Requires non empty
   */
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

static std::optional<size_t> read(const std::string &fname, std::unique_ptr<uint8_t[]> &readBuffer, const size_t readBufferSize)
{
  LOG << "Reading " << fname;
  std::FILE *file{ std::fopen(fname.c_str(), "rb") };
  if(file)
  {
    LOG << "  File open ok";
    FileGuard fguard{file};
    const auto bytesRead = std::fread(readBuffer.get(), 1, readBufferSize, file);
    if(const int error = std::ferror(file); error != 0)
    {
      LOG << "  Error [" << error << "] while reading " << fname;
    }
    else if(bytesRead == 0)
    {
      LOG << "  Empty file " << fname;
    }

    return bytesRead;
  }

  return std::nullopt;
}

int main(int argc, char *argv[])
{
  static constexpr size_t FILE_COUNT_LIMIT = 1'048'576;
  static constexpr size_t FNAME_MAX_SIZE = 451; // arbitrary value
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

  FnamesMemory fnamesArray = initFnamesMem(FILE_COUNT_LIMIT, FNAME_MAX_SIZE);
  if(!fnamesArray)
  {
    LOG << "Cant initizalize memory to hold file names";
    return 1;
  }

  auto readBuffer = allocBuffer(READ_BUFF_SIZE);
  if(!readBuffer)
  {
    LOG << "Cant initialize memory for reading";
    return 1;
  }

  std::FILE *outputFile = std::fopen(std::string{filename}.c_str(), "wb");
  if(!outputFile)
  {
    LOG << "Couldn't open merged.log for writing";
    return 1;
  }
  
  FileGuard writeFileGuard{outputFile};
//  char wbuf[32 * KB];
//  std::setvbuf(mergedLog, wbuf, _IOFBF, 32 * KB);

  auto writeBuffer = allocBuffer(READ_BUFF_SIZE);
  if(!writeBuffer)
  {
    LOG << "Can't initilize memory for writing.";
    return 1;
  }

  std::mutex fnamesMutex, bufferSwapMutex;
  std::atomic_bool finishedPathTraversal{false};
  std::atomic_bool finishedReading{false};
  std::condition_variable fnameAccess;
  std::condition_variable signalWrite;
  std::optional<size_t> bytesRead{std::nullopt};

  std::jthread writeThread([&]
  {
    const auto start = NOW();
    LOG << "Entering write thread";
    while(!finishedReading)
    {
      std::unique_lock swapLock(bufferSwapMutex);
      signalWrite.wait(swapLock, [&] { return bytesRead || finishedReading; });

      if(!bytesRead)
        break;

      const size_t bytesWritten = std::fwrite(writeBuffer.get(), 1, *bytesRead, outputFile);
      if(bytesWritten != bytesRead)
      {
        LOG << " Error [" << std::ferror(outputFile) << "] while writing. ABORTING!";
        std::abort();
        return;
      }

      LOG << " File write ok " << bytesWritten << " bytes";
//      fwrite(NEWLINE, 1, NEWLINE_LEN, mergedLog);

      bytesRead.reset();
    }

    LOG << "Finished write thread " << DURATION_S(start).count() << "s";
  });

  std::jthread readThread([&]
  {
    const auto start = NOW();
    LOG << "Entering read thread";
    while(!finishedPathTraversal)
    {
      std::unique_lock lock(fnamesMutex);
      fnameAccess.wait(lock, [&] { return !fnamesArray.empty() || finishedPathTraversal; });

      if(finishedPathTraversal)
        break;

      const std::string fname{fnamesArray.last()};
      fnamesArray.pop_last();

      lock.unlock();
      fnameAccess.notify_one();

      if(fname.empty())
        continue;

      if(const auto bytesCount = read(fname, readBuffer, READ_BUFF_SIZE))
      {
        std::lock_guard swapGuard(bufferSwapMutex);
        readBuffer.swap(writeBuffer);
        bytesRead = *bytesCount;
      }
      signalWrite.notify_one();
   //   LOG << "Buffers swapped";
   }

    // finishedPathTraversal == true
    // we have fnameArray for ourselves so no more locking fnameArray
    while(!fnamesArray.empty())
    {
  //    LOG << "No Locking copy";

      const std::string fname {fnamesArray.last()};
      fnamesArray.pop_last();

      if(const auto bytesCount = read(fname, readBuffer, READ_BUFF_SIZE))
      {
        std::lock_guard swapGuard(bufferSwapMutex);
        readBuffer.swap(writeBuffer);
        bytesRead = *bytesCount;
      }

      signalWrite.notify_one();
 //     LOG << "Buffers swapped";
    }

    finishedReading = true;
    signalWrite.notify_all();

    LOG << "Finished read thread " << DURATION_S(start).count() << "s";
  });

  const fs::path fsExtension{extension};
  const fs::path fsOutFilename{filename};
  for(auto itEntry = fs::recursive_directory_iterator("./");
      itEntry != fs::recursive_directory_iterator();
      ++itEntry)
  {
    const auto &path = itEntry->path();
    if(path.filename() == fsOutFilename || fs::is_directory(path) || path.extension() != fsExtension)
    {
      //LOG << "Ommiting " << path.filename().string();
      continue;
    }

    {
      std::lock_guard lock(fnamesMutex);
      fnamesArray.append(path.string());
//      LOG << "Added to name pool [" << fnamesArray.last() << "]";
    }
    fnameAccess.notify_one();
  }
  finishedPathTraversal = true;
  fnameAccess.notify_all();
  
  LOG << "Finished path traversal";

#if 0
  for(size_t i = 0; i < fnamesArray.count; ++i)
  {
    const auto val = fnamesArray.get(i);
    LOG << "Added to name pool [" << val << "] len [" << val.size() << ']';
  }
#endif

#if 0
  if(readThread.joinable() && writeThread.joinable())
  {
    readThread.join();
    writeThread.join();

    readBuffer.release();
    writeBuffer.release();
  }
#endif

  return 0;
}
