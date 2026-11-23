cmake_minimum_required(VERSION 3.19...3.29)

project(win32_file_hash VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(TARGET_NAME win32_file_hash)

add_executable(${TARGET_NAME}
  src/win32_file_hash.cpp
)

set_target_properties(
  ${TARGET_NAME}
  PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" # /MT flag
)

target_link_options(
  ${TARGET_NAME}
  PRIVATE
    $<$<CONFIG:RELEASE>:/INCREMENTAL:NO>
)

# target_link_libraries(
#   ${TARGET_NAME}
#   PRIVATE
#     msdelta
# )

target_compile_definitions(
  ${TARGET_NAME}
  PRIVATE
    UNICODE
    _UNICODE
    _WINDOWS
)
