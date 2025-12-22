#include "OrderBook.h"

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
    TradeVector trades;

    while (!sell_book.empty() && incoming->QtyRemaining() > 0) {
        auto best_sell_container = sell_book.begin();
        double best_sell_price = best_sell_container->first;

        if (best_sell_price > incoming->Price()) {
            break;
        }

        auto &sell_queue = best_sell_container->second;
        OrderPtr resting = sell_queue.front();

        uint32_t match_qty =
            std::min(incoming->QtyRemaining(), resting->QtyRemaining());

        trades.push_back(std::make_shared<Trade>(
            incoming->Timestamp(), incoming->Trader(), resting->Trader(),
            resting->Price(), match_qty));

        incoming->QtyRemaining(incoming->QtyRemaining() - match_qty);
        resting->QtyRemaining(resting->QtyRemaining() - match_qty);

        if (resting->QtyRemaining() == 0) {
            locators.erase(resting->OrderId());
            sell_queue.pop_front();
            if (sell_queue.empty()) {
                sell_book.erase(best_sell_container);
            }
        }
    }

    if (incoming->QtyRemaining() > 0) {
        AddToBuyBook(incoming);
    }

    return trades;
}

TradeVector OrderBook::MatchSell(OrderPtr incoming) {
    TradeVector trades;

    while (!buy_book.empty() && incoming->QtyRemaining() > 0) {
        auto best_buy_container = buy_book.begin();
        double best_buy_price = best_buy_container->first;

        if (best_buy_price < incoming->Price()) {
            break;
        }

        auto &buy_queue = best_buy_container->second;
        OrderPtr resting = buy_queue.front();

        uint32_t match_qty =
            std::min(incoming->QtyRemaining(), resting->QtyRemaining());

        trades.push_back(std::make_shared<Trade>(
            incoming->Timestamp(), resting->Trader(), incoming->Trader(),
            resting->Price(), match_qty));

        incoming->QtyRemaining(incoming->QtyRemaining() - match_qty);
        resting->QtyRemaining(resting->QtyRemaining() - match_qty);

        if (resting->QtyRemaining() == 0) {
            locators.erase(resting->OrderId());
            buy_queue.pop_front();
            if (buy_queue.empty()) {
                buy_book.erase(best_buy_container);
            }
        }
    }

    if (incoming->QtyRemaining() > 0) {
        AddToSellBook(incoming);
    }

    return trades;
}

void OrderBook::AddToSellBook(OrderPtr o) {
    sell_book[o->Price()].push_back(o);
    locators[o->OrderId()] = OrderLocator{SIDE::SELL, o->Price()};
}

void OrderBook::AddToBuyBook(OrderPtr o) {
    buy_book[o->Price()].push_back(o);
    locators[o->OrderId()] = OrderLocator{SIDE::BUY, o->Price()};
}

bool OrderBook::CancelOrder(uint64_t order_id) {
    auto it = locators.find(order_id);
    if (it == locators.end()) {
        return false;
    }

    const OrderLocator loc = it->second;

    if (loc.side == SIDE::BUY) {
        auto lvl = buy_book.find(loc.price);
        if (lvl == buy_book.end()) {
            locators.erase(it);
            return false;
        }

        auto &q = lvl->second;
        for (auto oq_it = q.begin(); oq_it != q.end(); ++oq_it) {
            if ((*oq_it)->OrderId() == order_id) {
                q.erase(oq_it);
                locators.erase(it);
                if (q.empty())
                    buy_book.erase(lvl);
                return true;
            }
        }
    } else {
        auto lvl = sell_book.find(loc.price);
        if (lvl == sell_book.end()) {
            locators.erase(it);
            return false;
        }

        auto &q = lvl->second;
        for (auto oq_it = q.begin(); oq_it != q.end(); ++oq_it) {
            if ((*oq_it)->OrderId() == order_id) {
                q.erase(oq_it);
                locators.erase(it);
                if (q.empty())
                    sell_book.erase(lvl);
                return true;
            }
        }
    }

    locators.erase(it);
    return false;
}
