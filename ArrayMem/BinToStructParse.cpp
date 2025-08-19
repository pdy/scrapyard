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

#include <iostream>
#include <memory>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <climits>

struct PackedType // no additional padding will be added by compiler
{
    int integer{0};
    uint32_t u32{0};
    uint8_t byte{0};
    char pad[3] = {0}; // manually add three bytes of padding at the end
};

struct NonPackedType // compilier will add padding
{
    int integer{0};
    uint8_t byte{0};
    // three bytes of padding will be added here
    uint32_t u32{0};
};

static void c_style_read_packed_type(uint8_t *buff, size_t buffSize)
{
    static constexpr size_t TYPE_SIZE = sizeof(PackedType);
    assert(buffSize >= TYPE_SIZE);

    auto ret = PackedType {
        .integer = -12,
        .u32 = 12,
        .byte = 128
    };

    std::memcpy(buff, &ret, TYPE_SIZE); // C++ legal, bytes will be placed in platform byte order, so Little Endian on most machines
}

static void c_style_read_non_packed_type(uint8_t *buff, size_t buffSize)
{
    static constexpr size_t TYPE_SIZE = sizeof(NonPackedType);
    assert(buffSize >= TYPE_SIZE);

    auto ret = NonPackedType {
        .integer = -12,
        .byte = 128,
        .u32 = 12
    };

    std::memcpy(buff, &ret, TYPE_SIZE); // C++ legal, bytes will be placed in platform byte order, so Little Endian on most machines
}

template
<
    typename T,

    // below is to ensure we can only use integral, non float types (int, unsigned, uint32_t etc) - C++11 compatible
    typename std::enable_if<std::is_integral<T>::value, int>::type = 0
>
T parseBytesLE(const uint8_t *raw, size_t offset = 0)
{
    assert(raw != nullptr && "Requires raw pointer to be non nullptr");

    static_assert(CHAR_BIT == 8, "Requires 8 bit byte");

    static constexpr size_t SIZE = sizeof(T);

    T ret{0};
    for(size_t i = offset, pos = SIZE - 1; i < offset + SIZE; ++i, --pos)
    {
      const size_t op = (SIZE - 1 - pos) * 8;
      ret |= static_cast<T>(raw[i]) << op;
    }

    return ret;
}

template<typename T>
struct AlignedHeap
{
    using type = T;
    static constexpr auto type_align = alignof(T);
    static constexpr size_t type_size = sizeof(T);
    static constexpr size_t memory_size = type_size + type_align - 1;

    std::unique_ptr<uint8_t[]> memory;
    uint8_t *aligned_ptr {nullptr};

    AlignedHeap()
    {
        memory = std::make_unique<uint8_t[]>(memory_size);
        std::memset(memory.get(), 0x00, memory_size);
        aligned_ptr = (uint8_t*)((intptr_t)memory.get() + (type_align - 1) & ~intptr_t(type_align - 1));
    }

    template<typename ...Args>
    T* create(Args&&... args) const
    {
        return new(aligned_ptr) T(std::forward<Args>(args)...);
    }
};

template<typename T>
struct AlignedHeap<T[]> // turn off array version
{};

int main()
{
    static constexpr size_t PACKED_TYPE_SIZE = sizeof(PackedType);
    static constexpr size_t NON_PACKED_TYPE_SIZE = sizeof(NonPackedType);
    static constexpr size_t PACKED_TYPE_ALIGN = alignof(PackedType);
    static constexpr size_t NON_PACKED_TYPE_ALIGN = alignof(NonPackedType);

    using STypeIntType = decltype(PackedType::integer);
    using STypeU32Type = decltype(PackedType::u32);
    using STypeByteType = decltype(PackedType::byte);

    uint8_t serialized_packed[PACKED_TYPE_SIZE] = {0};
    c_style_read_packed_type(serialized_packed, PACKED_TYPE_SIZE);

    uint8_t serialized_non_packed[NON_PACKED_TYPE_SIZE] = {0};
    c_style_read_non_packed_type(serialized_non_packed, NON_PACKED_TYPE_SIZE);

    alignas(NonPackedType) uint8_t aligned_stack[NON_PACKED_TYPE_SIZE] = {0};
    NonPackedType *nonPackedFromStack = new(aligned_stack) NonPackedType;
    c_style_read_non_packed_type(aligned_stack, NON_PACKED_TYPE_SIZE);

    AlignedHeap<NonPackedType> memory;
    NonPackedType *nonPackedFromHeap = memory.create();
    c_style_read_non_packed_type(memory.aligned_ptr, NON_PACKED_TYPE_SIZE);

    PackedType packedType{
        .integer = parseBytesLE<STypeIntType>(serialized_packed),
        .u32 = parseBytesLE<STypeU32Type>(serialized_packed, PACKED_TYPE_ALIGN),
        .byte = parseBytesLE<STypeByteType>(serialized_packed, 2 * PACKED_TYPE_ALIGN)
    };

    NonPackedType nonPackedType{
        .integer = parseBytesLE<STypeIntType>(serialized_non_packed),
        .byte = parseBytesLE<STypeByteType>(serialized_non_packed, NON_PACKED_TYPE_ALIGN),
        .u32 = parseBytesLE<STypeU32Type>(serialized_non_packed, 2 * NON_PACKED_TYPE_ALIGN)
    };

    std::cout << "PackedType:\n";
    std::cout << "int:     " << packedType.integer << '\n';
    std::cout << "u32:     " << packedType.u32 << '\n';
    std::cout << "byte:    " << static_cast<int>(packedType.byte) << '\n';

    std::cout << '\n';

    std::cout << "NonPackedType:\n";
    std::cout << "int:     " << nonPackedType.integer << '\n';
    std::cout << "u32:     " << nonPackedType.u32 << '\n';
    std::cout << "byte:    " << static_cast<int>(nonPackedType.byte) << '\n';

    std::cout << '\n';

    std::cout << "NonPackedType from stack:\n";
    std::cout << "int:     " << nonPackedFromStack->integer << '\n';
    std::cout << "u32:     " << nonPackedFromStack->u32 << '\n';
    std::cout << "byte:    " << static_cast<int>(nonPackedFromStack->byte) << '\n';

    std::cout << '\n';

    std::cout << "NonPackedType from heap:\n";
    std::cout << "int:     " << nonPackedFromHeap->integer << '\n';
    std::cout << "u32:     " << nonPackedFromHeap->u32 << '\n';
    std::cout << "byte:    " << static_cast<int>(nonPackedFromHeap->byte) << '\n';

    return 0;
}
