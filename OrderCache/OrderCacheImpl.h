#include "OrderCache.h"
#include <deque>
#include <vector>

/*
 *  OrderCacheInterface does not have virtual dtor
 *  so not really safe to inherit from it.
 *
 *  If it's oversight of some sort just use decalaration
 *  from "#if 0 #endif" block
 *
 */

class OrderCacheImpl
{

  std::deque<Order> m_cache;

public:

  // add order to the cache
  void addOrder(Order order);

  // remove order with this unique order id from the cache
  void cancelOrder(const std::string& orderId); 

  // remove all orders in the cache for this user
  void cancelOrdersForUser(const std::string& user); 

  // remove all orders in the cache for this security with qty >= minQty
  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty); 

  // return the total qty that can match for the security id
  unsigned int getMatchingSizeForSecurity(const std::string& securityId); 

  // return all orders in cache in a vector
  std::vector<Order> getAllOrders() const;  

};


#if 0
class OrderCacheImpl : public OrderCacheInterface
{

  std::deque<Order> m_cache;

public:

  // add order to the cache
  void addOrder(Order order) override;

  // remove order with this unique order id from the cache
  void cancelOrder(const std::string& orderId) override; 

  // remove all orders in the cache for this user
  void cancelOrdersForUser(const std::string& user) override; 

  // remove all orders in the cache for this security with qty >= minQty
  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override; 

  // return the total qty that can match for the security id
  unsigned int getMatchingSizeForSecurity(const std::string& securityId) override; 

  // return all orders in cache in a vector
  std::vector<Order> getAllOrders() const override;  

};
#endif
