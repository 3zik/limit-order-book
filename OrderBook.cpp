#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <functional>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>


enum class OrderType{
  GoodTillCancel,
  FillAndKill
};

enum class Side{
  Buy,
  Sell
};

using Price = std::int; // alias for readability
using Quantity = std::u_int32_t;
using OrderId = std::u_int64_t;

struct LevelInfo{
  Price price_;
  Quantity quantity_;
}

using LevelInfos = std::vector<LevelInfo>

class OrderbookLevelInfos{
public:
  OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
    : bids_{ bids }, asks_{ asks } { }

  const LevelInfos& GetBids() const { return bids_; }
  const LevelInfos& GetAsks() const { return asks_; }

private:
  LevelInfos bids_;
  LevelInfos asks_;
}

class Order{
public:
  Order(OrderType orderType, OrderId orderId, Side side, Price price, Quantity quantity)
    : orderType_ { orderType }
    , orderId_ { orderId }
    , side_ { side }
    , price {price}
    , initialQuantity_ { quantity }
    , remainingQuantity_ {quantity}
    { }

    OrderId_ GetOrderId() const { return orderId_; }
    Side GetSide() const { return side_; }
    Price GetPrice() const { return price_; }
    OrderType GetOrderType() const { return orderType_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Quantity GetFilledQuantity() const { return GetInitialQuantity() - GetRemainingQuantity(); }
    bool IsFilled() const { return GetRemainingQuantity() == 0; }
    void Fill(Quantity quantity){
      if (quantity > GetRemainingQuantity()) // safety checkj
        throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));
      
      remainingQuantity_ -= quantity;
    }
private:
  OrderType orderType_;
  OrderId orderId_;
  Side side_;
  Price price_;
  Quantity initialQuantity_;
  Quantity remainingQuantity_;
};

/* choose shared ptr because we want to use a signle Order in multiple data structures
 * we want refernece semantics
 * Order can be stored in orders dictionary, and can be stored in a bids/asks dictionary
 */

using OrderPointer = std::shared_ptr<Order>;
// Choose a list for now, can be potentially optimized with a std::vector
using OrderPointers = std::list<OrderPointer>;

/* modifying an order is more complicated than just adding/removing an order
* abstraction of an order to be modified. To actually modify, we cance and then replace.
* Cancel: get by Id. Replace requires price and quantity (and also side) so we can change them.
*/


class OrderModify{
public:
  OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
    : orderId_ { orderId }
    , side_ { side }
    , price_ { price }
    , quantity_ { quantity }
  { }
  
  OrderId GetOrderId() const { return orderId_; }
  Price GetPrice() const { return price_; }
  Side GetSide() const { return side_; }
  Quantity GetQuantity() const { return quantity_; }
 
  // converts an existing order into a new modified order
  OrderPointer ToOrderPointer(OrderType type) const {
    return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
  }

private:
  OrderId orderId_;
  Side side_;
  Price price_;
  Quantity quantity_;
};

struct TradeInfo{
  OrderId orderId_;
  Price price_;
  Quantity quantity_;
};

// Trade object is a representation of 2 tradeinfo objects: 1 bid, 1 ask

class Trade{
public:
  Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
    : bidTrade_ { bidTrade }
    , askTrade_ { askTrade }
  { }

  const TradeInfo& GetBidTrade() const { return bidTrade_; }
  const TradeInfo& GetAskTrade() const { return askTrade_; }

private:
  TradeInfo bidTrade_;
  TradeInfo askTrade_;
};

// we want a vector of trades because there can be many different trades to match

using Trades = std::vector<Trade>;

class OrderBook{
private:
  /* use a map to repr bids and ask.
   * bids sorted in descending order from best to worst
   * asks sorted in ascending order from best to worst
   * Have O(1) access to orders by id
   */

  struct OrderEntry{
    OrderPointer order_ { nullptr };
    OrderPointers::iterator location_;
  };

  // using std::greater, we store keys in descending order, less is oppo
  std::map<Price, OrderPointers, std::greater<Price>> bids_;
  std::map<Price, OrderPointers, std::less<Price>> asks_;

  std::unordered_map<OrderId, OrderEntry> orders_;

  // match method - e.g. non-f&Kill, add it to the orderbook,
  // can match method - e.g. fillAndKill is never added if it cant be matched, its just discarded
  
  bool CanMatch(Side side, Price price){
    if (side == Side::Buy){
      if (asks_.empty())
        return false;

      const auto& [bestAsk, _] = *asks_.begin(); // best ask, the level with the lowest pric
      // RHS is dereference the iterator, which is at .begin(), so the start of asks
      // LHS is auto of a pair, [pricelvl, (quantity, but we dont care abt it)]
      return price >= bestAsk;
    }
    else{
      if (bids_.empty())
        return false;
      
      const auto& [bestBid, _] = *bids.begin();
      return price <= bestBid;
    }
  }

  Trades MatchOrders(){
    Trades trades;
    // reserves memory in std::vecotr
    trades.reserve(orders_.size());
    
    while(true){
      if (bids_.empty() || asks_.empty()){
        break; // just break if no bids or asks
      } 
      
      auto& [bidPrice, bids] = *bids_.begin();
      auto& [askPrice, asks] = *asks_.begin();

      if(bidPrice < askPrice){
        break;
      }

      while(bids.size() && ask.size()){
        auto& bid = bids.front(); // TIME-PRICE PRIORIITY.  WE CHOOE the ONe that is there first (front of queue)
        auto& ask = asks.front();

        Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

        bid->Fill(quantity);
        ask->Fill(quantity);

        if (bid->IsFilled()){
          bids.pop_front();
          orders_erase(bid->GetOrderId());
        }

        if (ask->IsFilled){
          asks.pop_front();
          orders_.erase(ask->GetOrderId());
        }

        if(bids.empty()){
          bids_.erase(bidPrice);
        }
        if(asks.empty()){
          asks_.erase(askPrice);
        }

        trades.push_back(Trade {
            TradeInfo { bid->GetOrderId(), bid->GetPrice(), quantity },
            TradeInfo { ask->GetOrderId(), ask->GetPrice(), quantity }
            });
      }
    }
    

    if (!bids.empty()){
      auto& [_, bids] = *bids_.begin();
      auto& order = bids.front();
      if(order->GetOrderType() == OrderType::FillAndKill)
        CancelOrder(order->GetOrderId());
    }

    if (!asks.empty()){
      auto& [_, asks] = *asks_.begin();
      auto& order = asks.front();
      if (order->GetOrderType() == OrderType::FillAndKill)
        CancelOrder(order->getOrderId());
    }

    return trades;
  }
public:
  Trades AddOrder(OrderPointer order){
    if (!orders_contains(order->GetOrderId()) // .contains() in a Cpp 20+ feature
        return { }; // order Ids MUST BE UNIQUE
    
    if (order->GetOrderType() == OrderType::FillAndKill && !CanMatch(order->getSide(), order->GetPrice()))
      return { }; // return nothing when we are fillandkill but cant match
  
    OrderPointers::iterator iterator;

    if(order->GetSide() == Side::Buy){
      auto& orders = bids_[order->GetPrice()];
      orders.push_back(order);
      iterator = std::next(order.begin(), order.size()-1);
    }
    else{
      auto& orders = asks_[order->GetPrice()];
      orders.push_back(order);
      iterator = std::next(orders.begin(), orders.size() - 1);
    }
    
    orders_.insert({ order->getOrderId(), OrderEntry{ order, iterator } });
    return MatchOrders();
  }

  void CancelOrder(OrderId orderId){
    if(!orders_.contains(orderId))
      return ;

    const auto& [order, iterator] = orders_.at(orderId);
    orders_.erase(orderId);

    if(order->GetSide() == Side::Sell){
      auto price = order->GetPrice();
      auto& orders = asks_.at(price);
      orders.erase(iterator);
      if (orders.empty()){
        asks_.erase(price);
      }
    }
    else{
      auto price = order->GetPrice();
      auto& orders = bids_.at(price);
      orders.erase(iterator);
      if (orders.empty()){
        bids_.erase(price);
      }
    }
  }

  Trades MatchOrder(OrderModify order){
    if (!orders_contains(order.GetOrderId())){
      return { };
    }
    const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
    
    CancelOrder(order.GetOrderId());
    return AddOrder(order.ToOrderPointeR(existingOrder->GetOrderType()));
    
  }
  
  std::size_t Size() const { return orders_.size(); }

  OrderbookLevelInfos GetOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(orders_.size());
    askInfos.reserve(orders_.size());
    
    auto CreateLevelInfos = [](Price price, const OrderPointers& orders){
      return LevelInfo{ price, std::accumulate(orders.begin(), orders.end(), (Quantity)0, 
          [](std::size_t runningSum, const OrderPointer& order
          {return runningSum + order->GetRemainingQuantity(); }) };
    };

    for (const auto& [price, orders] : bids_){
    bidInfos.push_back(CreateLevelInfos(price, orders));
    }
    
    for (const auto& [price, orders] : asks_){
      askInfos.push_back(CreateLevelInfos(price, orders));
    }

    return OrderBookLevelInfos{ bidInfos, askInfos };
  }
  
};


int main(){
  
  return 0;
}


