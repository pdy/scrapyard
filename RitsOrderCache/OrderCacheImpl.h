#include "OrderCache.h"

class OrderCacheImpl : public OrderCacheInterface
{
public:
   ~OrderCacheImpl() = default;

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
