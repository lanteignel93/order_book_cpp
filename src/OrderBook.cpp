#include "orderbook/OrderBook.h"

#include <algorithm>
#include <iostream>

TradeVector OrderBook::ProcessOrder(const Order &_incoming) {
    OrderPtr incoming = std::make_shared<Order>(_incoming);

    if (incoming->Side() == SIDE::BUY) {
        return MatchBuy(incoming);
    } else {
        return MatchSell(incoming);
    }
}

TradeVector OrderBook::MatchBuy(OrderPtr incoming) {
    TradeVector trade_vector;

    while (!sell_book.empty() && incoming->QtyRemaining() > 0) {
        auto best_sell_it = sell_book.begin();
        double best_sell_price = best_sell_it->first;

        if (best_sell_price > incoming->Price()) {
            break;
        }

        auto &sell_queue = best_sell_it->second;
        OrderPtr resting = sell_queue.front();

        uint32_t match_qty =
            std::min(incoming->QtyRemaining(), resting->QtyRemaining());

        trade_vector.push_back(std::make_shared<Trade>(
            incoming->Timestamp(), incoming->Trader(), resting->Trader(),
            resting->Price(), match_qty));

        incoming->QtyRemaining(incoming->QtyRemaining() - match_qty);
        resting->QtyRemaining(resting->QtyRemaining() - match_qty);

        if (resting->QtyRemaining() == 0) {
            locators.erase(resting->OrderId());
            sell_queue.pop_front();
            if (sell_queue.empty()) {
                sell_book.erase(best_sell_it);
            }
        }
    }

    if (incoming->QtyRemaining() > 0) {
        AddToBuyBook(incoming);
    }

    return trade_vector;
}

TradeVector OrderBook::MatchSell(OrderPtr incoming) {
    TradeVector trade_vector;

    while (!buy_book.empty() && incoming->QtyRemaining() > 0) {
        auto best_buy_it = buy_book.begin();
        double best_buy_price = best_buy_it->first;

        if (best_buy_price < incoming->Price()) {
            break;
        }

        auto &buy_queue = best_buy_it->second;
        OrderPtr resting = buy_queue.front();

        uint32_t match_qty =
            std::min(incoming->QtyRemaining(), resting->QtyRemaining());

        trade_vector.push_back(std::make_shared<Trade>(
            incoming->Timestamp(), resting->Trader(), incoming->Trader(),
            resting->Price(), match_qty));

        incoming->QtyRemaining(incoming->QtyRemaining() - match_qty);
        resting->QtyRemaining(resting->QtyRemaining() - match_qty);

        if (resting->QtyRemaining() == 0) {
            locators.erase(resting->OrderId());
            buy_queue.pop_front();
            if (buy_queue.empty()) {
                buy_book.erase(best_buy_it);
            }
        }
    }

    if (incoming->QtyRemaining() > 0) {
        AddToSellBook(incoming);
    }

    return trade_vector;
}

void OrderBook::AddToSellBook(OrderPtr o) {
    auto &q = sell_book[o->Price()];
    q.push_back(o);

    auto it = std::prev(q.end());
    locators[o->OrderId()] = OrderLocator{SIDE::SELL, o->Price(), it};
}

void OrderBook::AddToBuyBook(OrderPtr o) {
    auto &q = buy_book[o->Price()];
    q.push_back(o);

    auto it = std::prev(q.end());
    locators[o->OrderId()] = OrderLocator{SIDE::BUY, o->Price(), it};
}

bool OrderBook::CancelOrder(uint64_t order_id) {
    auto loc_it = locators.find(order_id);
    if (loc_it == locators.end()) {
        return false;
    }

    const OrderLocator loc = loc_it->second;

    if (loc.side == SIDE::BUY) {
        auto lvl = buy_book.find(loc.price);
        if (lvl == buy_book.end()) {
            locators.erase(loc_it);
            return false;
        }

        auto &q = lvl->second;
        q.erase(loc.it);
        locators.erase(loc_it);

        if (q.empty())
            buy_book.erase(lvl);
        return true;

    } else {
        auto lvl = sell_book.find(loc.price);
        if (lvl == sell_book.end()) {
            locators.erase(loc_it);
            return false;
        }

        auto &q = lvl->second;
        q.erase(loc.it);
        locators.erase(loc_it);

        if (q.empty())
            sell_book.erase(lvl);
        return true;
    }

    return false;
}
