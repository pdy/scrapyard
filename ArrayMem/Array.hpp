#ifndef ARRAY_HPP_
#define ARRAY_HPP_

#include <cstddef>
#include <utility>

namespace detail {

template<typename T>
struct remove_const
{
  using type = T;
};

template<typename T>
struct remove_const<const T>
{
  using type = T;
};

template<typename T>
struct remove_const<volatile T>
{
  using type = T;
};

template<typename T>
using remove_const_t = typename remove_const<T>::type;

} // namespace detail

template<typename Type>
class ArrayView
{
  Type *m_ptr {nullptr};
  size_t m_size{0};

public:

  using iterator = detail::remove_const_t<Type>*;
  using const_iterator = const detail::remove_const_t<Type>*;
  
  ArrayView() = default;
  ArrayView(Type *ptr, size_t size) noexcept
    : m_ptr(ptr), m_size(size)
  {}
  
  ArrayView(const ArrayView &other) noexcept
    : m_ptr(other.m_ptr), m_size(other.m_size)
  {}

  template<size_t SIZE>
  ArrayView(Type (&other)[SIZE])
    : m_ptr(other), m_size(SIZE)
  {}

  size_t size() const { return m_size; }
  bool empty() const { return m_size == 0; }

  detail::remove_const_t<Type>* data() { return m_ptr; }
  const detail::remove_const_t<Type>* data() const { return m_ptr; }

  iterator begin() { return m_ptr; }
  const_iterator begin() const { return m_ptr; }
  
  iterator end() { return m_ptr + m_size; }
  const_iterator end() const { return m_ptr + m_size; }

  detail::remove_const_t<Type>& operator[](size_t idx) { return m_ptr[idx]; }
  const detail::remove_const_t<Type>& operator[](size_t idx) const { return m_ptr[idx]; }
  
  ArrayView<Type>& operator=(ArrayView other)
  {
    swap(*this, other);
    return *this;
  }

  friend void swap(ArrayView<Type> &lhs, ArrayView<Type> &rhs)
  {
    using std::swap;

    swap(lhs.m_ptr, rhs.m_ptr);
    swap(lhs.m_size, rhs.m_size);
  }
};

#endif
