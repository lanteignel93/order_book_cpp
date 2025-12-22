#pragma once
#include "Order.h"
#include "Trade.h"
#include <deque>
#include <functional>
#include <map>
#include <unordered_map>

class OrderBook {
  public:
    TradeVector ProcessOrder(const Order &_incoming);

  private:
    // This is a good place to define your order book data structures.
    // A container for buy orders and a container for sell orders would be
    // useful You'll need to insert new unmatched orders, and remove or update
    // orders that are matched so accessing the "First-In"
    // at the lowest price (for sells) and highest price (for buys) is
    // efficient."
    using OrderQueue = std::deque<OrderPtr>;

    std::map<double, OrderQueue> sell_book;
    std::map<double, OrderQueue, std::greater<double>> buy_book;

    TradeVector MatchBuy(OrderPtr incoming);
    TradeVector MatchSell(OrderPtr incoming);
    void AddToBuyBook(OrderPtr o);
    void AddToSellBook(OrderPtr o);

    bool CancelOrder(uint64_t order_id);

    struct OrderLocator {
        SIDE side;
        double price;
    };

    std::unordered_map<uint64_t, OrderLocator> locators;

  private:
    // Feel free to add helper methods here as needed
};
