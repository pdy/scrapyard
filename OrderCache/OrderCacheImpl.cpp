#include "OrderCacheImpl.h"
#include <algorithm>
#include <iterator>
#include <type_traits>

namespace{

template<typename Func>
void remove_if(std::deque<Order> &cache, Func &&comp)
{
  static_assert(std::is_invocable_v<Func, const Order&>, "Comparator has to be callable with const Order&");

  cache.erase(
    std::remove_if(cache.begin(), cache.end(), std::forward<Func>(comp)),
    cache.end()
  );
}

} // namespace

// add order to the cache
void OrderCacheImpl::addOrder(Order order)
{
  m_cache.push_back(std::move(order));
}

// remove order with this unique order id from the cache
void OrderCacheImpl::cancelOrder(const std::string& orderId)
{
  remove_if(m_cache, [&](const Order &o) { return o.orderId() == orderId; }); 
}

// remove all orders in the cache for this user
void OrderCacheImpl::cancelOrdersForUser(const std::string& user)
{
  remove_if(m_cache, [&](const Order &o) { return o.user() == user; }); 
}

// remove all orders in the cache for this security with qty >= minQty
void OrderCacheImpl::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
  remove_if(m_cache, [&](const Order &o) { return o.securityId() == securityId && o.qty() >= minQty; }); 
}

// return the total qty that can match for the security id
unsigned int OrderCacheImpl::getMatchingSizeForSecurity(const std::string& securityId)
{
  return 0;
}

// return all orders in cache in a vector
std::vector<Order> OrderCacheImpl::getAllOrders() const
{
  std::vector<Order> ret;
  ret.reserve(m_cache.size());

  std::copy(m_cache.begin(), m_cache.end(), std::back_inserter(ret));

  return ret;
}



