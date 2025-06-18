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

#include <cassert>
#include <cstring>
#include <filesystem>

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

struct FnamesMemory
{
  char *data {nullptr};
  size_t regionCount {0};

  const size_t maxCount {0};
  const size_t regionSize {0};

  explicit operator bool() const { return data != nullptr; }

  bool append(std::string_view str)
  {
    if(regionCount == maxCount)
      return false;

    char *ptr = data + regionSize * regionCount;
    std::memset(ptr, 0x00, regionSize);
    std::memcpy(ptr, str.data(), str.size());
    ++regionCount;

    return true;
  }

  std::string_view get(size_t idx) const
  {
    assert(idx < regionCount && "Buffer overflow");
    return data + idx * regionSize;
  }

  std::string_view last() const
  {
    return data + (regionCount - 1) * regionSize;
  }
};

static FnamesMemory initFnamesMem(size_t count, size_t regionSize)
{
  char *data = new(std::nothrow) char[count * regionSize];
  if(!data)
    return FnamesMemory{};

  return FnamesMemory{
    .data = data,
    .regionCount = 0,
    .maxCount = count,
    .regionSize = regionSize
  };
}

int main(int argc, char *argv[])
{
  static constexpr size_t FILE_COUNT_LIMIT = 1'048'576;
  static constexpr size_t FNAME_MAX_SIZE = 450;
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

//  LOG << "file limit " << FILE_COUNT_LIMIT;
//  LOG << "filename " << filename << " extension " << extension;

  FnamesMemory fnamesArray = initFnamesMem(FILE_COUNT_LIMIT, FNAME_MAX_SIZE);
  if(!fnamesArray)
  {
    LOG << "Cant initizalize memory to hold file names";
    return 1;
  }

  const fs::path fsExtension{extension};
  for(auto itEntry = fs::recursive_directory_iterator("./");
      itEntry != fs::recursive_directory_iterator();
      ++itEntry)
  {
    const auto &path = itEntry->path();
    if(fs::is_directory(path) || path.extension() != fsExtension)
    {
      LOG << "Ommiting " << path.filename().string();
      continue;
    }

    fnamesArray.append(path.string());
    LOG << "Added to name pool [" << fnamesArray.last() << "]";
  }

  return 0;
   
}
