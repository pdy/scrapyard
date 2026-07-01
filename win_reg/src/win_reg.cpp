#include <concepts>
#include <cstddef>
#include <functional>
#include <mutex>
#include <new>
#include <optional>
#include <filesystem>
#include <sstream>
#include <string>
#include <syncstream>
#include <thread>
#include <vector>
#include <iostream>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <errhandlingapi.h>

namespace fs = std::filesystem;

namespace {

template<std::integral T>
T cast(std::integral auto val)
{
  return static_cast<T>(val);
}

template<typename T, typename ...Types>
struct OneOfTypes
{
  static constexpr bool value = (std::is_same_v<T, Types> || ...);
};

template<typename T>
concept Integers = OneOfTypes<T, int, size_t, long, unsigned>::value;

template<Integers T>
std::optional<T> string_to_type(std::string_view str)
{
  if (str.empty())
    return std::nullopt;

  if (str.size() == 1)
    if (str[0] == '.' || str[0] == ',')
      return std::nullopt;

  T result = {};
  const char* last = str.data() + str.size();
  if (auto [p, ec] = std::from_chars(str.data(), last, result); p == last)
  {
    return result;
  }
  else if (*p == '.' || *p == ',')
  {
    T temp;
    if (auto [l, err] = std::from_chars(++p, last, temp); l == last)
      return result;
  }

  return std::nullopt;
}

template<typename ...Args>
bool one_of(std::string_view str, Args&& ...args)
{
  for(const auto &arg : { std::forward<Args>(args)... }) {
    if(str == arg)
      return true;
  }

  return false;
}

struct Args
{
  std::string_view search_root, reg_val, output_file;
};

void usage()
{
  const std::string_view key_example = R"(HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-12-1-2255359616-1283222993-1354843794-900585320\Components)";
  std::cout << std::format(
    "-r, --root - key root\n"
    "-v, --value - value string\n\n"
    "win_reg.exe -r \"{}\" -v \"some_val\" -o \"output_log.txt\"",
    key_example
  );
}

std::optional<Args> process_args(int argc, char *argv[])
{
  if(argc < 2) {
    return std::nullopt;
  }

  Args ret;

  for(size_t i = 1; i < static_cast<size_t>(argc); ++i) {
    const std::string_view arg{ argv[i] };
    if(one_of(arg, "-r", "--root") && i + 1 < argc)
      ret.search_root = argv[++i];
    else if(one_of(arg, "-v", "--value") && i + 1 < argc)
      ret.reg_val = argv[++i];
    else if(one_of(arg, "-o", "--output") && i + 1 < argc)
      ret.output_file = argv[++i];
  }

  bool ok = true;
  if(ret.search_root.empty()) {
    std::cout << "Missing -r argument\n";
    ok = false;
  }

  if(ret.reg_val.empty()) {
    std::cout << "Missing -v argument\n";
    ok = false;
  }

  std::cout << std::format(
      "\troot  = \"{}\"\n"
      "\tvalue = \"{}\"\n"
      "\tfile  = \"{}\"\n",
      ret.search_root, ret.reg_val, ret.output_file
  );

  if(!ok)
    return std::nullopt;

  return ret;
}

std::string Win32ErrStr(unsigned long errCode)
{
  // https://learn.microsoft.com/en-us/windows/win32/sysinfo/getting-system-information

  char sysMsg[MAX_PATH] = {'\0'};
  char* p = sysMsg;

  ::FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
         FORMAT_MESSAGE_IGNORE_INSERTS,
         nullptr, errCode,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         sysMsg, MAX_PATH, nullptr );

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

  return sysMsg;
}

template<typename Func>
class ScopeGuard final
{
  Func m_func;
public:
  [[nodiscard]] ScopeGuard(Func func)
    : m_func(std::move(func))
  {}

  ~ScopeGuard()
  {
    m_func();
  }
};

template<std::invocable Func>
[[nodiscard]] auto MakeScopeGuard(Func func)
{
  return ScopeGuard(std::move(func));
}

[[nodiscard]]
auto MakeHandleGuard(HANDLE handle)
{
  // HANDLE is alias for void* so we need copy
  return MakeScopeGuard([=] { CloseHandle(handle); });
}

[[nodiscard]]
auto MakeRegKeyGuard(HKEY key)
{
  return MakeScopeGuard([=] { RegCloseKey(key); });
}

DWORD ReturnLastErr(DWORD last_err, std::string_view msg = "")
{
  if(!msg.empty()) {
    std::cout << std::format("{} - {}", msg, Win32ErrStr(last_err));
  }
  else
    std::cout << Win32ErrStr(last_err);

  return last_err;
}

DWORD ReturnLastErr(std::string_view msg = "")
{
  const DWORD last_err = ::GetLastError();
  return ReturnLastErr(last_err, msg);
}

#if 0
bool SubkeyHasNeedle(HKEY hSub, const std::string &needle) {
  DWORD valueIndex = 0;

  char valueName[256];
  DWORD valueNameLen = 256;

  while (true) {
      valueNameLen = 256;

      DWORD valueType = 0;
      char dataBuf[1024];
      DWORD dataBufBytes = sizeof(dataBuf);

      LONG rc = RegEnumValueA(
          hSub,
          valueIndex,
          valueName,
          &valueNameLen,
          nullptr,
          &valueType,
          (LPBYTE)dataBuf,
          &dataBufBytes
      );

      if (rc == ERROR_NO_MORE_ITEMS) break;
      if (rc != ERROR_SUCCESS) { valueIndex++; continue; }

      if (valueType == REG_SZ || valueType == REG_EXPAND_SZ) {
          std::string s(dataBuf, dataBufBytes / sizeof(char));
          size_t nul = s.find('\0');
          if (nul != std::string::npos) s.resize(nul);

          if (s.find(needle) != std::wstring::npos) return true;
      }

      valueIndex++;
  }

  return false;
}
#endif

} // namespace

int main(int argc, char *argv[])
{
  const auto args = process_args(argc, argv);
  if(!args) {
    usage();
    return ERROR_BAD_ARGUMENTS;
  }

  const HANDLE proc_handle = ::GetCurrentProcess();

  HANDLE tokenHandle;
  if(0 == ::OpenProcessToken(proc_handle, TOKEN_READ, &tokenHandle))
    return ReturnLastErr("OpenProcessToken");

  const auto tokenHandleGuard = MakeHandleGuard(tokenHandle);

  HKEY hroot;
  LONG rc = RegOpenKeyExA(
      HKEY_LOCAL_MACHINE,
      std::string{args->search_root}.c_str(),
      0,
      KEY_READ | KEY_WOW64_64KEY,
      &hroot
  );

  if(rc != ERROR_SUCCESS)
    return ReturnLastErr(cast<DWORD>(rc), "RegOpenKeyExA");

  const auto root_guard = MakeRegKeyGuard(hroot);

#if 0
  const auto EndsWith = [](const std::wstring& s, const std::wstring& suffix) {
    if (suffix.size() > s.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), s.rbegin());
  };
#endif

  std::ofstream out_file;
  if(!args->output_file.empty()) {
    out_file.open(std::string{ args->output_file });
  }

 // Iterate subkeys under Components; check each subkey's values for a REG_SZ/REG_EXPAND_SZ
    // whose string equals/contains the needle.
    DWORD subkeyIndex = 0;
    char subkeyName[256];
    DWORD subkeyNameLen = 256;

    while (true) {
        subkeyNameLen = 256;
        rc = RegEnumKeyExA(
            hroot,
            subkeyIndex++,
            subkeyName,
            &subkeyNameLen,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        if (rc == ERROR_NO_MORE_ITEMS) break;
        if (rc != ERROR_SUCCESS) continue;

        HKEY hSub = nullptr;
        if (RegOpenKeyExA(hroot, subkeyName, 0, KEY_READ | KEY_WOW64_64KEY, &hSub) != ERROR_SUCCESS) {
            continue;
        }

        // Enumerate values in the subkey
        DWORD valueIndex = 0;
        char valueName[256];
        DWORD valueNameLen = 256;

        while (true) {
            valueNameLen = 256;

            DWORD valueType = 0;
            char dataBuf[512];
            DWORD dataBufBytes = sizeof(dataBuf);

            rc = RegEnumValueA(
                hSub,
                valueIndex++,
                valueName,
                &valueNameLen,
                nullptr,
                &valueType,
                (LPBYTE)dataBuf,
                &dataBufBytes
            );

            if (rc == ERROR_NO_MORE_ITEMS) break;
            if (rc != ERROR_SUCCESS) continue;

            const bool isString = (valueType == REG_SZ || valueType == REG_EXPAND_SZ);
            if (isString) {
                std::string s(dataBuf, dataBufBytes / sizeof(char));

                // Trim trailing null(s) if present
                const size_t nul = s.find('\0');
                if (nul != std::wstring::npos) s.resize(nul);

                // You asked "contains any entry with string value 'TestTest\\MyTest'".
                if (s.find(args->reg_val) != std::string::npos) {
                  std::ostringstream oss;
                  oss << "Match:\n"
                      << "  Subkey: " << subkeyName << "\n"
                      << "  Value:  " << valueName << "\n"
                      << "  Data:   " << s << "\n"
                      << "  Delete: ";



#if 0
                  const LONG del_rc = RegDeleteKeyA(hroot, subkeyName);
                  if(del_rc == ERROR_SUCCESS)
                    oss << "SUCCES!\n";
                  else
                    oss << "FAIL (" << Win32ErrStr(del_rc) << ")\n";
#endif

                  oss << "FAIL (Turned Off in source code)\n";

                  std::cout << oss.str();
                  if(out_file.is_open()) {
                    out_file << oss.str();
                  }

                  break;
                }
            }
        }

        RegCloseKey(hSub);
    }


  return 0;
}
