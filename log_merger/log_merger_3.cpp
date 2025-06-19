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
#include "xxhash.hpp"

#include <cstdint>
#include <functional>
#include <new>
#include <queue>
#include <thread>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <unordered_set>
#include <vector>

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

using thread_count_t = std::invoke_result_t<decltype(std::thread::hardware_concurrency)>;
namespace fs = std::filesystem;
using namespace std::chrono_literals;

struct AppArgs
{
  std::string_view filename;
  std::string_view searchExtension;
};

static void usage()
{
  LOG << "!!!!! log_merger_2 is more reliable !!!!!";
  LOG << "!!!!! This one requires a lot of    !!!!!";
  LOG << "!!!!! RAM and is not synced well    !!!!!";
  LOG << "!!!!! with I/O                      !!!!!";

  LOG << "log_merger_3 -f <file name> -e <extension>";
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

struct BioGuardDeleter
{
  void operator()(BIO *bio) const
  {
    BIO_free_all(bio);
  }
};

using BIO_uptr = std::unique_ptr<BIO, BioGuardDeleter>;

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

static uint8_t *allocBuffer(size_t size)
{
  return new(std::nothrow) uint8_t[size];
}

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

static std::vector<uint8_t> hashFile(const std::string &path, const EVP_MD *evpMd)
{
  BIO_uptr bioRaw{ BIO_new_file(path.c_str(), "rb") };
  if(!bioRaw)
    return {};

  // mdtmp will be freed with bio
  BIO *mdtmp = BIO_new(BIO_f_md());
  if(!mdtmp)
    return {};

  // WTF OpenSSL?
  // Every EVP_<digest>() function returns const pointer, but
  // BIO_set_md which supposed to consume this pointer takes.... non const!
  // WTF OpenSSL?
  BIO_set_md(mdtmp, const_cast<EVP_MD*>(evpMd));
  BIO_uptr bio{ BIO_push(mdtmp, bioRaw.release()) };
  if(!bio)
    return {};

  {
    char buf[10240];
    int rdlen;
    do {
      char *bufFirstPos = buf;
      rdlen = BIO_read(bio.get(), bufFirstPos, sizeof(buf));
    } while (rdlen > 0);
  }

  uint8_t mdbuf[EVP_MAX_MD_SIZE];
  const int mdlen = BIO_gets(mdtmp, reinterpret_cast<char*>(mdbuf), EVP_MAX_MD_SIZE);

  return std::vector<uint8_t>(std::begin(mdbuf), std::next(std::begin(mdbuf), mdlen));
}

struct FileHash
{
  std::vector<uint8_t> hash;

  FileHash(std::vector<uint8_t> &&other) noexcept
    : hash{std::move(other)}
  {}

  FileHash() = default;

  FileHash(const FileHash &other) noexcept
    : hash{other.hash}
  {}

  FileHash(FileHash &&other) noexcept
    : hash{std::move(other.hash)}
  {}

  bool operator==(const FileHash &other) const { return hash == other.hash; }
  bool operator!=(const FileHash &other) const { return !(*this == other); }

  FileHash& operator=(std::vector<uint8_t> bt)
  {
    hash = std::move(bt);
    return *this;
  }

  FileHash& operator=(FileHash other) noexcept
  {
    swap(*this, other);
    return *this;
  }

  friend void swap(FileHash &lhs, FileHash &rhs) noexcept
  {
    using std::swap;
    swap(lhs.hash, rhs.hash);
  }
};

struct HashFileHash
{
  std::uint64_t operator()(const FileHash &filehash) const
  {
    return xxh::xxhash<64>(filehash.hash);
  }
};

using FileHashCache = std::unordered_set<FileHash, HashFileHash>;

class FileHashThreadPool final
{
  // state for output file names
  FnamesMemory &m_outBuff;
  std::mutex &m_outBuffMutex;
  std::condition_variable &m_signalFileAdded;
  std::atomic_bool &m_pathTraversalFinished;

  // state for hash caching
  FileHashCache m_hashCache;
  std::mutex m_hashCacheMutex;

  // state for threading
  thread_count_t m_threadCount{0};
  std::queue <std::string> m_filePaths = {};
  std::mutex m_filePathsMutex = {};
  std::condition_variable m_pathAvailable = {};
  std::atomic_bool m_running{false};
  std::unique_ptr<std::jthread[]> m_threads = nullptr;

public:
  FileHashThreadPool() = delete;
  FileHashThreadPool(const FileHashThreadPool&) = delete;
  FileHashThreadPool(FileHashThreadPool&&) = delete;

  FileHashThreadPool(FnamesMemory &outbuff, std::mutex &outbuffMutex, std::condition_variable &signalFileAdded, std::atomic_bool &pathTraversalFinished)
    : m_outBuff{outbuff}, m_outBuffMutex{outbuffMutex}, m_signalFileAdded{signalFileAdded}, m_pathTraversalFinished{pathTraversalFinished}
  {}

  ~FileHashThreadPool()
  {
    stop();
  }

  bool reserve(size_t count)
  {
    try{
      m_hashCache.reserve(count);
    }catch(const std::bad_alloc&)
    {
      return false;
    }

    return true;
  }

  void start(thread_count_t threadCount = 3)
  {
    stop();
    createThreads(threadCount);
  }

  void stop()
  {
    if(m_running)
    {
      m_running = false;
      m_pathAvailable.notify_all();
      joinThreads();
    }
  }

  void joinThreads()
  {
    for (thread_count_t i = 0; i < m_threadCount; ++i)
    {
      if (m_threads[i].joinable())
      {
        m_threads[i].join();
      }
    }

    m_threadCount = 0;
  }


  void schedule(std::string filepath)
  {
    {
      std::lock_guard lock(m_filePathsMutex);
      m_filePaths.push(std::move(filepath));
    }
    m_pathAvailable.notify_one();
  }

  void notify()
  {
    m_pathAvailable.notify_all();
  }

private:
  void fileHashWorker()
  {
    const auto start = NOW();
    while(m_running)
    {
      std::unique_lock lock(m_filePathsMutex);
      m_pathAvailable.wait(lock, [this] { return !m_filePaths.empty() || !m_running || m_pathTraversalFinished; });
      
      if(m_pathTraversalFinished && m_filePaths.empty())
        break;

      if(m_running)
      {
        const auto file = std::move(m_filePaths.front());
        m_filePaths.pop();
        lock.unlock();

        auto fileHash = hashFile(file, EVP_blake2b512());
        LOG << "  " << file << ' ' << bin2Hex(fileHash);

        bool inserted = false;
        {
          std::lock_guard lock(m_hashCacheMutex);
          const auto [iter, ins] = m_hashCache.insert(std::move(fileHash));
          inserted = ins;
        }

        if(inserted)
        {
          {
            std::lock_guard lock(m_outBuffMutex);
            m_outBuff.append(file);
          }
          LOG << "  Added unique " << file;
          m_signalFileAdded.notify_one();
        }
      }
    }
    LOG << "Hash worker finished " << DURATION_MS(start).count() << "ms"; //std::this_thread::get_id()
  }

  std::string bin2Hex(const std::vector<uint8_t> &buff)
  {
    std::ostringstream oss;
    for(const auto bt : buff){
      oss << std::setfill('0') << std::setw(2) << std::hex << +bt;
    }
    return oss.str();
  }

  void createThreads(thread_count_t threadCount)
  {
    m_threads = std::make_unique<std::jthread[]>(threadCount);
    m_threadCount = threadCount;
    m_running = true;
    for (thread_count_t i = 0; i < threadCount; ++i)
    {
      m_threads[i] = std::jthread([this, w = &FileHashThreadPool::fileHashWorker]{ std::invoke(w, this); });
    }
  }

};

class FileWriteThreadPool
{
  // state for writing
  std::mutex m_fileMutex;
  std::FILE &m_outputFile;
  std::unique_ptr<uint8_t[]> m_buff_1;
  std::unique_ptr<uint8_t[]> m_buff_2;
  static constexpr size_t READ_BUFF_SIZE = 2 * GB + 10; // +10 just in case
                                                      
  // state for input buffer
  FnamesMemory &m_fnamesArray;
  std::mutex &m_fnamesMutex;
  std::condition_variable &m_fnamesSignal;
  std::atomic_bool &m_finishedHashing;

  // state for threading
  thread_count_t m_threadCount{0};
  std::queue <std::string> m_filePaths = {};
  std::mutex m_filePathsMutex = {};
  std::condition_variable m_pathAvailable = {};
  std::atomic_bool m_running{false};
  std::unique_ptr<std::jthread[]> m_threads = nullptr;


public:
  FileWriteThreadPool(std::FILE &outputFile, FnamesMemory &fnamesArray, std::mutex &fnamesMutex, std::condition_variable &fnamesSignal, std::atomic_bool &finishedHashing)
    : m_outputFile{outputFile},
      m_fnamesArray{fnamesArray},
      m_fnamesMutex{fnamesMutex},
      m_fnamesSignal{fnamesSignal},
      m_finishedHashing{finishedHashing}
  {}

  bool reserve()
  {
    m_buff_1.reset(std::move(allocBuffer(READ_BUFF_SIZE)));
    if(!m_buff_1)
      return false;

    m_buff_2.reset(std::move(allocBuffer(READ_BUFF_SIZE)));
    if(!m_buff_2)
      return false;

    return true;
  }

  void start()
  {
    stop();
    createThreads();
  }

  void stop()
  {
    if(m_running)
    {
      m_running = false;
      m_pathAvailable.notify_all();
      joinThreads();
    }
  }

  void joinThreads()
  {
    for (thread_count_t i = 0; i < m_threadCount; ++i)
    {
      if (m_threads[i].joinable())
      {
        m_threads[i].join();
      }
    }

    m_threadCount = 0;
  }

private:

  void worker(std::unique_ptr<uint8_t[]> &buffer)
  {
    const auto start = NOW();

    while(m_running)
    {
      std::unique_lock lock(m_fnamesMutex);
      m_fnamesSignal.wait(lock, [&] { return !m_fnamesArray.empty() || !m_running || m_finishedHashing; });

      if(!m_running || (m_fnamesArray.empty() && m_finishedHashing))
        break;

      const std::string fileName{ m_fnamesArray.last() };
      m_fnamesArray.pop_last();

      lock.unlock();

      if(const auto bytesRead = read(fileName, buffer, READ_BUFF_SIZE))
      {
        // TODO: This part propably needs to be cut into BUFSIZ chunks

        std::lock_guard lokc(m_fileMutex);
        std::fwrite(buffer.get(), 1, *bytesRead, &m_outputFile);
      }
    }

    LOG << "Write worker finised " << DURATION_MS(start).count() << "ms";
  }

  void createThreads()
  {
    m_threads = std::make_unique<std::jthread[]>(2);
    m_threadCount = 2;
    m_running = true;
    
    m_threads[0] = std::jthread(
      [this, w = &FileWriteThreadPool::worker](std::unique_ptr<uint8_t[]> &buff)
        { std::invoke(w, this, buff); },
      std::ref(m_buff_1)    
    );
    
    m_threads[1] = std::jthread(
      [this, w = &FileWriteThreadPool::worker](std::unique_ptr<uint8_t[]> &buff)
        { std::invoke(w, this, buff); },
      std::ref(m_buff_2)    
    );
  }

};

int main(int argc, char *argv[])
{
  static constexpr size_t FILE_COUNT_LIMIT = 1'048'576;
  static constexpr size_t FNAME_MAX_SIZE = 451; // arbitrary value
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
#if 0
  auto readWriteBuffer = allocBuffer(READ_BUFF_SIZE * 2);
  if(!readWriteBuffer)
  {
    LOG << "Cant initialize memory for reading";
    return 1;
  }
#endif

  std::FILE *outputFile = std::fopen(std::string{filename}.c_str(), "wb");
  if(!outputFile)
  {
    LOG << "Couldn't open merged.log for writing";
    return 1;
  }
  
  FileGuard writeFileGuard{outputFile};
//  char wbuf[32 * KB];
//  std::setvbuf(mergedLog, wbuf, _IOFBF, 32 * KB);


  std::mutex fnamesMutex;
  std::condition_variable fnamesSignal;
  std::atomic_bool finishedPathTraversal{false};
  std::atomic_bool finishedHashing{false};

  FileHashThreadPool hasher(fnamesArray, fnamesMutex, fnamesSignal, finishedPathTraversal);
  if(!hasher.reserve(FILE_COUNT_LIMIT))
  {
    LOG << "Couldn't initialize enough memory for hash cache";
    return 1;
  }
  hasher.start(5);

  FileWriteThreadPool writer(*outputFile, fnamesArray, fnamesMutex, fnamesSignal, finishedHashing);
  if(!writer.reserve())
  {
    LOG << "Couldn't initialize enough memory for I/O";
    return 1;
  }

  writer.start();
  LOG << "Writer threads started";

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

    hasher.schedule(path.string());
  }

  finishedPathTraversal = true; 
  LOG << "Finished path traversal";

  hasher.notify();
  hasher.joinThreads();
  finishedHashing = true;
  fnamesSignal.notify_all();
  LOG << "Finished hashing";

  writer.joinThreads();
  LOG << "Finished writing";

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
