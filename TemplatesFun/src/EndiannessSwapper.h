#ifndef ENDIANESS_SWAPPER_H_
#define ENDIANESS_SWAPPER_H_

#include <type_traits>

#include "application/TrivialLogger.h"

#include <iostream>

template<class T,
    typename = typename std::enable_if<std::is_integral<T>::value && !std::is_floating_point<T>::value>::type>
auto templfunc(T var) -> decltype(var)
{
    return var;
}

static std::vector<uint8_t> toBytes()
{
    return {0xaa, 0xbb};
}

template<class T>
static std::vector<uint8_t> operate(T&& bytes)
{
    std::cout << "is lvalue " << std::is_lvalue_reference<T>::value << "\n";
    std::reverse(bytes.begin(), bytes.end());
    return std::forward<T>(bytes);
}

static void runTest()
{
    operate(toBytes());
}

inline uint16_t swapEndiannes(uint16_t val)
{
    return static_cast<uint16_t>( (val << 8) | (val >> 8) );
}

inline uint32_t swapEndiannes(uint32_t val)
{
    return (
            ((val << 24) & 0xff000000)
          | ((val <<  8) & 0x00ff0000)
          | ((val >>  8) & 0x0000ff00)
          | ((val >> 24) & 0x000000ff)
          );
}

#endif
