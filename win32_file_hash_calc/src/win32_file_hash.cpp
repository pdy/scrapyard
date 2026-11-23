#include <chrono>
#include <concepts>
#include <expected>
#include <filesystem>
#include <iterator>
#include <queue>
#include <fstream>
#include <type_traits>
#include <print>
#include <iostream>
#include <array>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Wincrypt.h>
#include <shellapi.h>
#include <errhandlingapi.h>
#include <strsafe.h>

static constexpr size_t BUFSIZE = 8192;
static constexpr size_t SHA256LEN = 32;

#define NOW() (std::chrono::high_resolution_clock::now())
#define DURATION_S(start) (std::chrono::duration_cast<std::chrono::seconds>(NOW() - start))
#define DURATION_MS(start) (std::chrono::duration_cast<std::chrono::milliseconds>(NOW() - start))
#define DURATION_US(start) (std::chrono::duration_cast<std::chrono::microseconds>(NOW() - start))

namespace fs = std::filesystem;

namespace {

template<typename T>
concept Invocable = std::is_invocable_v<T>;

template <Invocable T>
class ScopeGuard
{
  T m_func;

public:
  [[nodiscard]] ScopeGuard(T callable)
    : m_func{std::move(callable)}
  {}

  ~ScopeGuard()
  {
    m_func();
  }
};

template<Invocable T>
[[nodiscard]] auto MakeScopeGuard(T callable)
{
  return ScopeGuard<T>(std::move(callable));
}

[[nodiscard]] auto MakeHandleGuard(HANDLE handle)
{
  // HANDLE is alias for void* so we need copy
  return MakeScopeGuard([=] { CloseHandle(handle); });
}

[[nodiscard]] auto MakeContextGuard(HCRYPTPROV handle)
{
  // HCRYPTPROV is alias for ULONG_PTR so we need copy
  return MakeScopeGuard([=] { CryptReleaseContext(handle, 0); });
}

[[nodiscard]] auto MakeHashGuard(HCRYPTHASH hash)
{
  // HCRYPTPROV is alias for ULONG_PTR so we need copy
  return MakeScopeGuard([=] { CryptDestroyHash(hash); });
}

template<std::integral T>
T cast(std::integral auto num)
{
  return static_cast<T>(num);
}

template<typename ...Args>
struct OneOf
{
  static constexpr bool value = (std::is_same_v<Args, char> || ...);
};

template<typename T>
concept CharType = OneOf<T, char, wchar_t>::value;

template<CharType CharT = char>
std::basic_string<CharT> Win32ErrStr(unsigned long errCode/*, std::wstring_view msg*/)
{
  // https://learn.microsoft.com/en-us/windows/win32/sysinfo/getting-system-information

  CharT sysMsg[MAX_PATH] = {0};
  CharT* p = sysMsg;

  if constexpr(std::is_same_v<CharT, char>)
  {
    ::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
           FORMAT_MESSAGE_IGNORE_INSERTS,
           nullptr, errCode,
           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
           sysMsg, MAX_PATH, nullptr );
  }
  else if(std::is_same_v<CharT, wchar_t>)
  {
    ::FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM |
           FORMAT_MESSAGE_IGNORE_INSERTS,
           nullptr, errCode,
           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
           sysMsg, MAX_PATH, nullptr );
  }

  // Trim the end of the line and terminate it with a null
  // 9 - \t (horizontal tab)
  // [0 - 32) - All characters in this area excepting 9
  // 46 - . (dot)
  while (*p++)
  {
    if ((*p != 9 && *p < 32) || *p == 46)
    {
      *p = 0;
      break;
    }
  }

  //return std::format(L"{} failed with error {} ({})", msg, errCode, sysMsg);;
  return sysMsg;
}

std::wstring Win32ErrWStr(unsigned long error_code)
{
  return Win32ErrStr<wchar_t>(error_code);
}

void ByteToChars(uint8_t bt, char &lhs, char &rhs)
{
  static constexpr char CHARS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  lhs = CHARS[static_cast<size_t>(bt >> 4)];
  rhs = CHARS[static_cast<size_t>(bt & 0x0f)];
}

std::string WideToAnsi(std::wstring_view wide)
{
  if(wide.empty())
    return {};

  const int strLen = WideCharToMultiByte(
      CP_THREAD_ACP,
      0,          // dwFlags composition check
      wide.data(),
      cast<int>(wide.size()), // ccWideChar - size in characters, we omit last null zero
      nullptr,    // lpMultiByteStr - pointer to output buffer
      0,          // cbMultiByte - output buffer size
      nullptr,    // lpDefaultChar - pointer to the character to use if character cannot be represented in specfied code page
      nullptr     // lpUsedDefaultCharacter - pointer to flag to indicate if default character has been used
  );

  std::string ret;
  ret.resize(strLen);
  WideCharToMultiByte(CP_THREAD_ACP, 0, wide.data(), cast<int>(wide.size()), ret.data(), cast<int>(ret.size()), nullptr, nullptr);

  return ret;
}

#if 0
std::expected<std::vector<uint8_t>, std::wstring> ReadEntireFile(const wchar_t* path) {
  // https://learn.microsoft.com/en-us/windows/win32/fileio/opening-a-file-for-reading-or-writing

  HANDLE file = ::CreateFileW(path,                   // file to open
                              GENERIC_READ,           // open for reading
                              FILE_SHARE_READ,        // share for reading
                              nullptr,                // default security
                              OPEN_EXISTING,          // existing file only
                              FILE_ATTRIBUTE_NORMAL,  // normal file
                              nullptr);

  if (file == INVALID_HANDLE_VALUE)
    return std::unexpected(
        std::format(L"Can't open the file [{}], error [{}]", path, Win32ErrWStr(::GetLastError())));

  const auto handle_guard = MakeHandleGuard(file);

  const DWORD file_size = ::GetFileSize(file, nullptr);
  if (file_size == INVALID_FILE_SIZE) {
    const auto last_error = ::GetLastError();
    if (last_error != ERROR_SUCCESS)
      return std::unexpected(
        std::format(L"Can't read size of file [{}] error [{}]", path, Win32ErrWStr(last_error)));
    else
      return std::unexpected(std::format(L"Can't read size of file [{}]", path));
  }

  std::vector<uint8_t>ret;
  ret.resize(cast<size_t>(file_size));
  DWORD bytes_read = 0;
  if (FALSE == ::ReadFile(file, ret.data(), cast<int>(file_size), &bytes_read, nullptr))
    return std::unexpected(
        std::format(L"Can't read file [{}] error [{}]", path, Win32ErrWStr(::GetLastError())));

  return ret;
}
#endif

std::expected<std::vector<uint8_t>, std::wstring> ReadEntireFile(const wchar_t* path, LONGLONG filesize)
{
  HANDLE file = ::CreateFileW(path,                   // file to open
                              GENERIC_READ,           // open for reading
                              FILE_SHARE_READ,        // share for reading
                              nullptr,                // default security
                              OPEN_EXISTING,          // existing file only
                              FILE_ATTRIBUTE_NORMAL,  // normal file
                              nullptr);

  if (file == INVALID_HANDLE_VALUE)
    return std::unexpected(
        std::format(L"Can't open the file [{}], error [{}]", path, Win32ErrWStr(::GetLastError())));

  const auto file_guard = MakeHandleGuard(file);

  std::vector<uint8_t>ret;
  ret.resize(cast<size_t>(filesize));
  DWORD bytes_read = 0;
  if (FALSE == ::ReadFile(file, ret.data(), cast<int>(filesize), &bytes_read, nullptr))
    return std::unexpected(
        std::format(L"Can't read file [{}] error [{}]", path, Win32ErrWStr(::GetLastError())));

  return ret;
}

std::expected<std::array<uint8_t, SHA256LEN>, std::string> HashFileSHA256(HCRYPTPROV crypto_provider_ctx, const wchar_t *filepath)
{
  HANDLE file_handle = CreateFileW(filepath,
    GENERIC_READ,
    FILE_SHARE_READ,
    nullptr,
    OPEN_EXISTING,
    FILE_FLAG_SEQUENTIAL_SCAN,
    nullptr);

  if (INVALID_HANDLE_VALUE == file_handle)
  {
    return std::unexpected(std::format("CreateFileW failed {}", Win32ErrStr(::GetLastError())));
  }

  const auto file_handle_guard = MakeHandleGuard(file_handle);

  HCRYPTHASH hash_handle = 0;
  if (!CryptCreateHash(crypto_provider_ctx, CALG_SHA_256, 0, 0, &hash_handle))
  {
    return std::unexpected(std::format("CryptCreateHash failed: {}", Win32ErrStr(::GetLastError())));
  }

  const auto format = MakeHashGuard(hash_handle);

  BOOL read_file_result = FALSE;
  DWORD read_count = 0;
  uint8_t file_bytes[BUFSIZE] = {0};
  while ((read_file_result = ReadFile(file_handle, file_bytes, BUFSIZE, &read_count, nullptr)))
  {
      if (0 == read_count)
        break;

      if (!CryptHashData(hash_handle, file_bytes, read_count, 0))
      {
        return std::unexpected(std::format("CryptHashData failed: {}", Win32ErrStr(::GetLastError())));
      }
  }

  if (!read_file_result)
  {
    return std::unexpected(std::format("ReadFile failed: {}", Win32ErrStr(::GetLastError())));
  }

  std::array<uint8_t, SHA256LEN> out;
  DWORD hash_len = cast<DWORD>(out.size());
  if(!CryptGetHashParam(hash_handle, HP_HASHVAL, out.data(), &hash_len, 0))
  {
    return std::unexpected(std::format("CryptGetHashParam failed: {}", Win32ErrStr(::GetLastError())));
  }

  return out;
}

void FormatSingleArray(const std::array<uint8_t, SHA256LEN> &arr, std::string &out)
{
  out.clear();
  out += "std::array<uint8_t, 32> {{ ";
  for (size_t i = 0; i < arr.size(); ++i) {
    char lhs, rhs;
    ByteToChars(arr[i], lhs, rhs);
    if(i < arr.size() - 1)
      std::format_to(std::back_inserter(out), "0x{}{},", lhs, rhs);
    else
      std::format_to(std::back_inserter(out), "0x{}{} }}}},", lhs, rhs);
  }
}

bool IsDirectory(const wchar_t *path)
{
  const DWORD attribs = GetFileAttributesW(path);
  return (attribs != INVALID_FILE_ATTRIBUTES) && (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

} // namespace

int main()
{
  // https://learn.microsoft.com/en-us/windows/win32/seccrypto/example-c-program--creating-an-md-5-hash-from-file-content
  // https://learn.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory

  LPWSTR *argv {nullptr};
  int argc = -1;
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if(!argv)
  {
    std::println("CommandLineToArgvW failed");
    return 0;
  }

  if(argc != 3)
  {
    std::println(R"(
        !!! Incorrect number of arguments !!!

        win32_file_hash_calc <search_folder> <output_file>
    )");
    return ERROR_BAD_ARGUMENTS;
  }

  const wchar_t *arg_search_directory = argv[1];

  size_t length_of_arg = 0;
  StringCchLengthW(arg_search_directory, MAX_PATH, &length_of_arg);
  if(length_of_arg > MAX_PATH - 1)
  {
    std::wcout << "Directory path is too long [" << arg_search_directory << "]\n";
    return ERROR_BAD_ARGUMENTS;
  }

  if(!IsDirectory(arg_search_directory))
  {
    std::wcout << "\n\"" << arg_search_directory << "\" is not a directory\n";
    return ERROR_BAD_ARGUMENTS;
  }

  std::ofstream out_file(argv[2]);
  if(!out_file.is_open()) {
    std::wcout << "Can't open output file \"" << argv[2] << "\"\n";
    return ERROR_BAD_ARGUMENTS;
  }

  HCRYPTPROV crypt_provider_ctx = 0;
  if (!CryptAcquireContext(&crypt_provider_ctx, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
  {
    const auto status = GetLastError();
    std::println("CryptAcquireContext failed: {}", Win32ErrStr(status));

    return status;
  }

  const auto ctx_guard = MakeContextGuard(crypt_provider_ctx);

  out_file << "std::array array {\n";

  size_t total_folder_count = 1, total_file_count = 0, total_files_errors = 0;
  std::queue<std::wstring> dirs;
  dirs.push(arg_search_directory);
  std::string single_array_buffer;
  single_array_buffer.reserve(2048);
  for(; !dirs.empty(); dirs.pop())
  {
    const auto &curr_dir = dirs.front();
    std::wcout << "Searching \"" << curr_dir << "\"\n";

    WIN32_FIND_DATA find_file_data;
    HANDLE find = FindFirstFileW(std::format(L"{}\\*", curr_dir.c_str()).c_str(), &find_file_data);
    if (INVALID_HANDLE_VALUE == find)
    {
      const DWORD status = ::GetLastError();
      std::wcout << std::format(L"FindFirstFileW [{}] failed: {}", curr_dir, Win32ErrWStr(status));
      continue;
    }

    const auto find_guard = MakeHandleGuard(find);

    do {
      const std::wstring_view dir_name { find_file_data.cFileName };
      if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if (dir_name == L"." || dir_name == L"..")
          continue;

        //std::wcout << std::format(L"\tFound \"{}\"\n", dir_name);
        auto tmp = std::format(L"{}\\{}", curr_dir, find_file_data.cFileName);
        //std::wcout << "\tAdding \"" << tmp << "\"\n";
        dirs.push(std::move(tmp));
        ++total_folder_count;
      }
      else
      {
        ++total_file_count;

        LARGE_INTEGER filesize;
        filesize.LowPart = find_file_data.nFileSizeLow;
        filesize.HighPart = find_file_data.nFileSizeHigh;

        const auto fp = std::format(L"{}\\{}", curr_dir, dir_name);
        // std::wcout << "\tRead file \"" << fp << "\"\n";
        const auto file_content = ReadEntireFile(fp.c_str(), filesize.QuadPart);
        if(!file_content) {
          ++total_files_errors;
          std::wcout << "\t\t" << file_content.error() << L'\n';
          continue;
        }

        const auto hash_bytes = HashFileSHA256(crypt_provider_ctx, fp.c_str());
        if(!hash_bytes) {
          ++total_files_errors;
          std::cout << "\t\t" << hash_bytes.error() << "\n";
          continue;
        }

        FormatSingleArray(*hash_bytes, single_array_buffer);

        out_file << "  // " << WideToAnsi(fp) << '\n';
        out_file << std::format("  {}\n\n", single_array_buffer);
      }

    } while(FindNextFileW(find, &find_file_data));
  }

  out_file << "};";
  out_file.close();

  std::println("\nSearched {} folders. Hashed {}/{} files.", total_folder_count, total_file_count - total_files_errors, total_file_count);

  return ERROR_SUCCESS;
}
