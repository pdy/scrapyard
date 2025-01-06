#include "OrderCacheImpl.h"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <unordered_set>
#include <vector>

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
  // Had to make some assumtpion here, since...
  //
  // "SecId3 has only one Buy order, no other orders to match against"
  //
  // Assumption 1 - return 0 in such case as there is no match
  //
  // Doc makes it confusing which values are supposed to be accounted for resulting matching qty.
  //
  // Once it's Buy, once it's Sell. For example:
  // 
  // "Buy order can match against multiple Sell orders (and vice versa)
  //          - eg a security id "ABCD" has 
  //              Buy  order with qty 10000
  //              Sell order with qty  2000
  //              Sell order with qty  1000               
  //          - security id "ABCD" has a total match of 3000"
  // 
  // So it looks like we count Sell, but later such dataset:
  //
  //  Order("OrdId1",  "SecId1", "Sell", 100, "User10", "Company2"),
  //  Order("OrdId3",  "SecId1", "Buy",  300, "User13", "Company2"),
  //  Order("OrdId7",  "SecId1", "Sell", 700, "User10", "Company2"),
  //  Order("OrdId8",  "SecId1", "Sell", 800, "User2",  "Company1"),
  //  Order("OrdId11", "SecId1", "Sell",1100, "User13", "Company2"),
  //  Order("OrdId13", "SecId1", "Sell",1300, "User1",  "Company")
  //  
  // is supposed to return 300 cause OrdId3 Buy 300 matches with OrdId8 Sell 800 and OrdId13 Sell 1300.
  // So it looks like we count Buy.
  //  
  // But then, such dataset: 
  //
  //  Order("OrdId4",  "SecId2", "Sell", 400, "User12", "Company2"),
  //  Order("OrdId9",  "SecId2", "Buy",  900, "User6",  "Company2"),
  //  Order("OrdId10", "SecId2", "Sell",1000, "User5",  "Company1"),
  //  Order("OrdId12", "SecId2", "Buy", 1200, "User9",  "Company2")
  //
  // is supposed to return 1000 as OrdId9 Buy 900 matches with OrdId10 Sell 1000.
  //
  // So we count Sell here... 
  //
  // But then we count Buy in two more instances
  //  
  // here:
  //  Order("OrdId2",  "SecId3", "Sell", 200, "User8",  "Company2")
  //  Order("OrdId5",  "SecId3", "Sell", 500, "User7",  "Company2")
  //  Order("OrdId6",  "SecId3", "Buy",  600, "User3",  "Company1")
  // 
  // for total of 600
  //
  // and here:
  //  Order("OrderId2", "SecId2", "Sell", 3000, "User2", "CompanyB"),
  //  Order("OrderId4", "SecId2", "Buy",   600, "User4", "CompanyC"),
  //  Order("OrderId5", "SecId2", "Buy",   100, "User5", "CompanyB"),
  //  Order("OrderId7", "SecId2", "Buy",  2000, "User7", "CompanyE"),
  //  Order("OrderId8", "SecId2", "Sell", 5000, "User8", "CompanyE")
  //
  // for total of 2700
  //
  // If which specific side is supposed to be accounted for is volatile and depending of some circumstances,
  // then it's not specified clearly enough.
  //
  // Assumption 2 - since Buy is counted more often than Sell (3:2) I'm going to sum Buy.
  //

  

  //---------------------------------
  // This can be easly avoided by keeping a hash of sec id to list/table of all opearations
  // Task did not specify if we should be extra optimal with particular methods. 
  std::vector<const Order*> secOps;
  for(const auto &o : m_cache)
  {
    if(o.securityId() == securityId)
      secOps.push_back(&o);
  }
  //----------------------------------

  std::unordered_set<const Order*> used;

  // this is also unnecessary as we can sum directly to resulting buffer without additional order caching
  // but decided to left it as visualize the loop a little bit better
  // only thing we need is hash table of orders we already used
  //
  std::vector<std::vector<const Order*>> matches; 
  matches.emplace_back();
  size_t matchIdx = 0;
  for(size_t i = 0; i < secOps.size(); ++i)
  {
    const Order *o = secOps[i];
    if(used.find(o) != used.end())
      continue;

    auto &match = matches[matchIdx];
    match.push_back(o);

    for(size_t j = 0; j < secOps.size(); ++j)
    {
      if(i == j)
        continue;

      const auto *candidate = secOps[j];
      if(used.find(candidate) != used.end() || (candidate->company() == o->company() && candidate->user() != o->user()))
        continue;


      if(o->side() == "Sell" && candidate->side() == "Buy")
      {
        match.push_back(candidate);
      }
      else if(o->side() == "Buy" && candidate->side() == "Sell")
      {
        match.push_back(candidate);
      }
    }
    
    if(match.size() == 1)
    {
      match.clear();
    }
    else
    {
      for(const auto &m : match)
        used.insert(m);

      matches.emplace_back();
      ++matchIdx;
    }

  }

  if(matches.empty())
    return 0;

  //--------------------------------------------
  // this could be done already in two loops above
  std::vector<unsigned> ret(matches.size()); 
  size_t i = 0;
  for(const auto &m : matches)
  {
    for(const auto *o : m)
    {
      if(o->side() == "Buy")
        ret[i] += o->qty();
    }
   
    ++i;
  }
  //-------------------------------------------


  return std::accumulate(ret.begin(), ret.end(), 0u);
}

// return all orders in cache in a vector
std::vector<Order> OrderCacheImpl::getAllOrders() const
{
  std::vector<Order> ret;
  ret.reserve(m_cache.size());

  std::copy(m_cache.begin(), m_cache.end(), std::back_inserter(ret));

  return ret;
}



