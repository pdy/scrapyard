#ifndef OPTIONAL_HPP_
#define OPTIONAL_HPP_

#include <memory>
#include <type_traits>
#include <assert.h>

namespace internal {

template<typename T>
using non_const_t = typename std::remove_const<T>::type;

template<typename T>
struct is_noexcept_swappable
{
  static constexpr bool value = noexcept(swap(std::declval<T&>(), std::declval<T&>()));
};

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
    : value{std::move(val)}, engaged{true}
  {}
};

template<typename T>
struct storage_trivial_dtor : storage_base<T>
{
  using Base = storage_base<T>;
  using Base::Base ;

  ~storage_trivial_dtor() = default;

};

template<typename T>
struct storage_non_trivial_dtor : storage_base<T>
{
  using Base = storage_base<T>;
  using Base::Base;

  ~storage_non_trivial_dtor()
  {
    if(Base::engaged)
      Base::value.T::~T();
  }
};

template<typename T>
using optional_storage = typename std::conditional<
    std::is_trivially_destructible<non_const_t<T>>::value,
    storage_trivial_dtor<non_const_t<T>>,
    storage_non_trivial_dtor<non_const_t<T>>>::type;

} // namespace internal




template<typename T>
class Optional
{
  internal::optional_storage<T> m_storage;

  constexpr const T* get() const { return std::addressof(m_storage.value); }
  internal::non_const_t<T>* get() { return std::addressof(m_storage.value); }

  template<typename ...Args>
  void init(Args&& ...args)
  {
    assert(!m_storage.engaged);

    ::new(static_cast<void*>(get())) T(std::forward<Args>(args)...);
    m_storage.engaged = true;
  }

public:
  Optional() = default; 

  constexpr Optional(const T &val) noexcept
    : m_storage(val)
  {}

  constexpr Optional(T &&val) noexcept
    : m_storage(std::move(val))
  {}

  Optional(const Optional<T> &other)
    : m_storage()
  {
    if(other.has_value())
    {
      ::new(static_cast<void*>(get())) T(*other);
      m_storage.engaged = true;
    }
  }

  Optional(Optional<T> &&other)
    : m_storage()
  {
    if(other.has_value())
    {
      ::new(static_cast<void*>(get())) T(std::move(*other));
      m_storage.engaged = true;
    }
  }

  ~Optional() = default;
 
  const T& operator*() const & { assert(has_value()); return m_storage.value; }
  T& operator*() & { assert(has_value()); return m_storage.value; }

  T&& operator*() && { assert(has_value()); return std::move(m_storage.value); }

  constexpr explicit operator bool() const noexcept { return m_storage.engaged; }
  constexpr bool has_value() const noexcept { return m_storage.engaged; }
  
  const T& value() const & { return **this; }
  T& value() & { return **this; }

  T&& value() && { return std::move(**this); }

  template<typename U = internal::non_const_t<T>>
  T value_or(U &&u) const&
  {
    if(has_value())
      return **this;
   
    return std::forward<U>(u);
  }

  template<typename U = internal::non_const_t<T>>
  T&& value_or(U &&u) &&
  {
    if(has_value())
      return std::move(**this);
   
    return std::forward(u);
  }

  void reset() noexcept
  {
    if(has_value())
      get()->T::~T();
    
    m_storage.engaged = false;
  }

  Optional<T>& operator=(Optional<T> other) noexcept(internal::is_noexcept_swappable<Optional<T>>::value)
  {
    swap(*this, other);
    return *this;
  }

  friend void swap(Optional<T> &lhs, Optional<T> &rhs) noexcept(std::is_nothrow_move_constructible<T>::value)
  {
    if(lhs.has_value() && rhs.has_value())
    {
      using std::swap;
      swap(*lhs, *rhs);
    }
    else if(lhs.has_value() && !rhs.has_value())
    {
      rhs.init(std::move(*lhs));

      lhs.reset();
    }
    else if(!lhs.has_value() && rhs.has_value())
    {
      lhs.init(std::move(*rhs));
      rhs.reset();
    }

  }  
};

#endif
