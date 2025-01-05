/*
*  MIT License
*  
*  Copyright (c) 2020 Pawel Drzycimski
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

#include <algorithm>
#include <cmdline.h> 
#include "simplelog/simplelog.h"

#include "OrderCacheImpl.h"

inline bool operator == (const Order &lhs, const Order &rhs)
{
  return lhs.orderId() == rhs.orderId()
    && lhs.company() == rhs.company()
    && lhs.qty() == rhs.qty()
    && lhs.securityId() == rhs.securityId()
    && lhs.side() == rhs.side()
    && lhs.user() == rhs.user();
}

static void print(const std::vector<Order> &order)
{
  for(const auto &o : order)
    LOG << o.orderId() << " " << o.securityId() << " " << o.side() << " " << o.qty() << " " << o.user() << " " << o.company();
}

static void testCase_1()
{

  LOG << "Test case 1";
  const std::vector<Order> ORDERS{
    Order("OrderId1", "SecId1", "Buy",  1000, "User1", "CompanyA"),
    Order("OrderId2", "SecId2", "Sell", 3000, "User2", "CompanyB"),
    Order("OrderId3", "SecId1", "Sell",  500, "User3", "CompanyA"),
    Order("OrderId4", "SecId2", "Buy",   600, "User4", "CompanyC"),
    Order("OrderId5", "SecId2", "Buy",   100, "User5", "CompanyB"),
    Order("OrderId6", "SecId3", "Buy",  1000, "User6", "CompanyD"),
    Order("OrderId7", "SecId2", "Buy",  2000, "User7", "CompanyE"),
    Order("OrderId8", "SecId2", "Sell", 5000, "User8", "CompanyE")
  };

  OrderCacheImpl cache;
  for(const auto &o : ORDERS)
    cache.addOrder(o);


  const auto allOrders = cache.getAllOrders();

  LOG << "  getAllOrders " << (std::equal(ORDERS.begin(), ORDERS.end(), allOrders.begin(), allOrders.end()) ? "Ok" : "Fail!");


  // cancelOrderId
  {
    const std::vector<Order> copy{
      Order("OrderId1", "SecId1", "Buy",  1000, "User1", "CompanyA"),
      Order("OrderId2", "SecId2", "Sell", 3000, "User2", "CompanyB"),
      Order("OrderId4", "SecId2", "Buy",   600, "User4", "CompanyC"),
      Order("OrderId5", "SecId2", "Buy",   100, "User5", "CompanyB"),
      Order("OrderId6", "SecId3", "Buy",  1000, "User6", "CompanyD"),
      Order("OrderId7", "SecId2", "Buy",  2000, "User7", "CompanyE"),
      Order("OrderId8", "SecId2", "Sell", 5000, "User8", "CompanyE")
    };

    cache.cancelOrder("OrderId3");
    const auto actual = cache.getAllOrders();
    LOG << "  cancelOrder " << (std::equal(copy.begin(), copy.end(), actual.begin(), actual.end()) ? "Ok" : "Fail!");
  }

  // cancelOrdersForUser
  {
    const std::vector<Order> copy{
      Order("OrderId1", "SecId1", "Buy",  1000, "User1", "CompanyA"),
      Order("OrderId2", "SecId2", "Sell", 3000, "User2", "CompanyB"),
      Order("OrderId5", "SecId2", "Buy",   100, "User5", "CompanyB"),
      Order("OrderId6", "SecId3", "Buy",  1000, "User6", "CompanyD"),
      Order("OrderId7", "SecId2", "Buy",  2000, "User7", "CompanyE"),
      Order("OrderId8", "SecId2", "Sell", 5000, "User8", "CompanyE")
    };

    cache.cancelOrdersForUser("User4");
    const auto actual = cache.getAllOrders();
    LOG << "  cancelOrdersForUser " << (std::equal(copy.begin(), copy.end(), actual.begin(), actual.end()) ? "Ok" : "Fail!");
  }

  // cancelOrdersforSecIdWithMinQty
  {
    const std::vector<Order> copy{
      Order("OrderId1", "SecId1", "Buy",  1000, "User1", "CompanyA"),
      Order("OrderId5", "SecId2", "Buy",   100, "User5", "CompanyB"),
      Order("OrderId6", "SecId3", "Buy",  1000, "User6", "CompanyD"),
      Order("OrderId7", "SecId2", "Buy",  2000, "User7", "CompanyE"),
    };


    cache.cancelOrdersForSecIdWithMinimumQty("SecId2", 3000);
    const auto actual = cache.getAllOrders();
    LOG << "  cancelOrdersForSecIdWithMinimumQty " << (std::equal(copy.begin(), copy.end(), actual.begin(), actual.end()) ? "Ok" : "Fail!");
  }

}

static void testCase_2()
{
  LOG << "Test case 2 getMatchingSizeForSecurity";

  const std::vector<Order> ORDERS{
    //
    //Order("OrdId1",  "SecId1", "Sell", 100, "User10", "Company2"),
    Order("OrdId2",  "SecId3", "Sell", 200, "User8",  "Company2"),
    //Order("OrdId3",  "SecId1", "Buy",  300, "User13", "Company2"),
    Order("OrdId4",  "SecId2", "Sell", 400, "User12", "Company2"),
    Order("OrdId5",  "SecId3", "Sell", 500, "User7",  "Company2"),
    Order("OrdId6",  "SecId3", "Buy",  600, "User3",  "Company1"),
    //Order("OrdId7",  "SecId1", "Sell", 700, "User10", "Company2"),
    //Order("OrdId8",  "SecId1", "Sell", 800, "User2",  "Company1"),
    Order("OrdId9",  "SecId2", "Buy",  900, "User6",  "Company2"),
    Order("OrdId10", "SecId2", "Sell",1000, "User5",  "Company1"),
    //Order("OrdId11", "SecId1", "Sell",1100, "User13", "Company2"),
    Order("OrdId12", "SecId2", "Buy", 1200, "User9",  "Company2"),
    //Order("OrdId13", "SecId1", "Sell",1300, "User1",  "Company")
  };

  struct Expected { std::string secId; unsigned totalQy; };

  const std::vector<Expected> expected{
#if 0
    Expected{
      "SecId1", 300
    },
#endif
    Expected{
      "SecId2", 1000
    },
    Expected{
      "SecId3", 600
    }
  };


  OrderCacheImpl cache;
  for(const auto &o : ORDERS)
    cache.addOrder(o);

  
  for(const auto &e : expected)
  {
    const auto actual = cache.getMatchingSizeForSecurity(e.secId);

    LOG << "  " << e.secId << " " << e.totalQy << " == " << actual << " " << (e.totalQy == actual ? "Ok" : "Fail!");
  }

}

int main()
{
  testCase_1();
  testCase_2();
  return 0;
}

#if 0

int main(int argc, char *argv[])
{
  cmdline::parser arg;
  arg.add("help", 'h', "Print help.");
//  arg.add<std::string>("file", 'f', "Example file argument.", true);
    
  if(!arg.parse(argc, const_cast<const char* const*>(argv)))
  {
    const auto fullErr = arg.error_full();
    if(!fullErr.empty())
      LOG << fullErr;
     
    LOG << arg.usage();
    return 0;
  }
  
  if(arg.exist("help"))
  {
    LOG << arg.usage();
    return 0;
  } 

  /* Example file arg check and get
  if(!arg.exist("file"))
  {
    LOG << "--file or -f argument is mandatory!\n";
    LOG << arg.usage();
    return 0;
  }
  
  const std::string file = arg.get<std::string>("file");
  */
       
  return 0;
}

#endif
