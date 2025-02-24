#ifndef OPTIONAL_HPP_
#define OPTIONAL_HPP_

#include <type_traits>
//#include <utility>

namespace internal {

template <typename T>
inline constexpr typename std::remove_reference<T>::type&& constexpr_move(T&& t) noexcept
{
  return static_cast<typename std::remove_reference<T>::type&&>(t);
}

template<typename T>
struct storage_base
{
  union {
    char dummy;
    T value;
  };

  bool engaged;

  explicit constexpr storage_base() noexcept
    : dummy{0}, engaged{false}
  {}

  explicit constexpr storage_base(const T &val) noexcept
    : value{val}, engaged{true}
  {}

  explicit constexpr storage_base(T &&val) noexcept
    : value{constexpr_move(val)}, engaged{true}
  {}
};

template<typename T, bool U>
struct storage {};

template<typename T>
struct storage<T, true> : storage_base<T>
{
  using Base = storage_base<T>;
  using Base::Base ;

  ~storage() = default;

};

template<typename T>
struct storage<T, false> : storage_base<T>
{
  using Base = storage_base<T>;
  using Base::Base;

  ~storage()
  {
    if(Base::engaged)
      Base::value.T::~T();
  }
};

template<typename T>
struct optional_storage : storage<T, std::is_trivially_destructible<T>::value>
{
  using Base = storage<T, std::is_trivially_destructible<T>::value>;
  using Base::Base;
};

} // namespace internal




template<typename T>
class Optional
{
  internal::optional_storage<T> m_storage;

public:
  Optional() = default; 

  constexpr Optional(const T &val) noexcept
    : m_storage(val)
  {}

  constexpr Optional(T &&val) noexcept
    : m_storage(internal::constexpr_move(val))
  {
  }

  ~Optional() = default;
 
  const T& operator*() const { return m_storage.value; }
  T& operator*() { return m_storage.value; }

  constexpr explicit operator bool() const { return m_storage.engaged; }
  constexpr bool has_value() const { return m_storage.engaged; }
  
  const T& value() const { return **this; }
  T& value() { return **this; }
  template<typename U = std::remove_cv_t<T>>
  T value_or(U &&u) const
  {
    if(has_value())
      return **this;
   
    return u;
  }


#if 0
  void swap(Optional<T> &other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>)
  {
    using std::swap;
    if(m_storage.engaged && other.m_storage.engaged)
      ;
    else if(m_storage.engaged)
      ;
    else if(other.m_storage.engaged)
      ;

  }  
#endif
};

#endif
