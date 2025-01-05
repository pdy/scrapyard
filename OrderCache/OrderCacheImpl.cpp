#include "OrderCacheImpl.h"
#include <algorithm>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <unordered_map>
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

struct UniqueUser
{
  struct Match
  {
    enum class Type
    {
      Buy,
      Sell,
      InProgress
    };

//      Match() { orders.reserve(10); }

    Match(Type type_, const Order *order)
     : type{type_}
    {
      orders.reserve(10);
      orders.push_back(order);
    } 

    Type type;
    std::vector<const Order*> orders;

    unsigned total() const
    {
      if(type == Type::InProgress || orders.empty())
        return 0;

      unsigned ret = 0;
      size_t start = 0, end = 0;
      if(type == Type::Buy)
      {
        ret = orders[0]->qty();
        start = 1;
        end = orders.size();
      }
      else
      { // type sell
        ret = orders.back()->qty();
        start = 0;
        end = orders.size() - 1;
      } 

      unsigned value = 0;
      for(size_t i = start; i < end; ++i)
        value += orders[i]->qty();

      return ret - value;
    }
  };

  UniqueUser() noexcept = default;

  UniqueUser(UniqueUser &&other) noexcept
  {
    swap(*this, other);
  }

  UniqueUser(const UniqueUser &other) noexcept
  {
    id = other.id;
    matches = other.matches;
  }

  UniqueUser(const std::string &id_) noexcept
    : id{id_}
  {
    matches.reserve(10);
  }
    
  std::string id;
  std::vector<Match> matches;

  UniqueUser& operator=(UniqueUser other) noexcept
  {
    swap(*this, other);
    return *this;
  }

  friend void swap(UniqueUser &lhs, UniqueUser &rhs) noexcept
  {
    using std::swap;

    swap(lhs.id, rhs.id);
    swap(lhs.matches, rhs.matches);
  }

  void match(const Order *o)
  {
    if(matches.empty())
    {
      Match::Type newMatchType = o->side() == "Buy" ? Match::Type::Buy : Match::Type::InProgress;
      matches.emplace_back(newMatchType, o);
      return;
    }

    Match::Type type = o->side() == "Buy" ? Match::Type::Buy : Match::Type::Sell;
    if(type == Match::Type::Sell)
    {
      // if it's sell it can match if last is Buy or InProgress
      
      auto &last = matches.back();
      if(last.type == Match::Type::Buy || last.type == Match::Type::InProgress)
        last.orders.push_back(o);
      else
        matches.emplace_back(type, o);
    }
    else // type == Match::Type::Buy
    {
      // if it's Buy it can only match with InProgress, then it changes to Sell
      // new match otherwise
      
      auto &last = matches.back();
      if(last.type == Match::Type::InProgress)
      {
        last.orders.push_back(o);
        last.type = Match::Type::Sell;
      }
      else
      {
        matches.emplace_back(type, o);
      }

    }
  }

  unsigned int totalQty() const
  {
    unsigned ret = 0;

    for(const auto &m : matches)
      ret += m.total();

    return ret;
  }

};

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
  // "Can only match a Buy order with a Sell order"
  // "Buy order can match against multiple Sell orders (and vice versa)"
  // These two contradict each other. Vice versa means matching Sell with multiple Buy, but
  // Buy cannot match with another Buy?
  //
  // Assumption 1 - Buy can match with multiple Sells, mutliple Sells can match with later SINGLE Buy.
  //
  // "Users in the same company cannot match against each other." So...
  // What if two users in the same company under same secId have separate matching operations of their own?
  // For which user should I return total qty?
  //
  // Assumption 2 - Return the sum of total, separate matches of each user, as they fall under same secId.
  //
  // "Some orders may not match entirely or at all"
  // So ignore them?
  //
  // Assumption 3 - Ignoring non matching orders.
  //
  // It's also technically possible under description and assumptions that total will come up negative,
  // but I have no means to return that.

  

#if 0
  std::unordered_map<std::string, UniqueUser> secIdUsers;

  for(const auto &o : m_cache)
  {
    if(o.securityId() != securityId)
      continue;

    const auto id = o.user() + "_" + o.company();
    auto foundUser = secIdUsers.find(id);
    if(foundUser != secIdUsers.end())
    {
      foundUser->second.match(&o);
    }
    else
    {
      UniqueUser user{id};
      user.match(&o);
      secIdUsers[id] = std::move(user);
    }
  }


  unsigned int ret = 0;
  for(const auto &seciduser : secIdUsers)
    ret += seciduser.second.totalQty();
  
  return ret;

#endif

  std::vector<const Order*> secOps;
  for(const auto &o : m_cache)
  {
    if(o.securityId() == securityId)
      secOps.push_back(&o);
  }


  std::unordered_set<const Order*> used;
  std::vector<std::vector<const Order*>> matches;
  /*auto &match =*/ matches.emplace_back();
  size_t matchIdx = 0;
  for(size_t i = 0; i < secOps.size(); ++i)
  {
    const Order *o = secOps[i];
    if(used.find(o) != used.end())
      continue;

    auto &match = matches[matchIdx];
    match.push_back(o);

    std::unordered_set<const Order*> tmpUsed;
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
        tmpUsed.insert(candidate);
      }
      else if(o->side() == "Buy" && candidate->side() == "Sell")
      {
        match.push_back(candidate);
        tmpUsed.insert(candidate);
      }
    }
    
    if(match.size() == 1)
    {
      match.clear();
    }
    else
    {
      used.insert(o);
      for(const auto &tmp : tmpUsed)
        used.insert(tmp);

      /*match =*/ matches.emplace_back();
      ++matchIdx;
    }

  }

  if(matches.empty())
    return 0;

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



