#include "orderbook/OrderBook.h"

#include <cstdlib>
#include <iostream>
#include <string>

static int failures{0};

static void CHECK(bool cond, const std::string &msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << "\n";
        ++failures;
    } else {
        std::cout << "[PASS] " << msg << "\n";
    }
}

static void test_cancel_prevents_match() {
    std::cout << "\n=== test_cancel_prevents_match ===\n";

    OrderBook book;

    {
        Order buy1(1, 1, "Biden", SIDE::BUY, 100.0, 10);
        auto trade_vector = book.ProcessOrder(buy1);
        CHECK(trade_vector.empty(), "Resting BUY produces no trade_vector");
    }

    {
        bool cancel_status = book.CancelOrder(1);
        CHECK(cancel_status, "CancelOrder(1) returns true");
    }

    {
        Order sell1(2, 2, "Donald", SIDE::SELL, 100.0, 5);
        auto trade_vector = book.ProcessOrder(sell1);

        CHECK(trade_vector.empty(), "SELL after cancellation produces 0 trade_vector");

        if (!trade_vector.empty()) {
            std::cout << "Unexpected trade_vector:\n";
            for (const auto &t : trade_vector) {
                std::cout << " ts=" << t->Timestamp() << " buyer=" << t->Buyer()
                          << " seller=" << t->Seller()
                          << " price=" << t->Price() << " qty=" << t->Qty()
                          << "\n";
            }
        }
    }
}

static void test_fifo_same_price_sell_side() {
    std::cout << "\n=== test_fifo_same_price_sell_side ===\n";

    OrderBook book;

    {
        Order sell1(1, 1, "Donald", SIDE::SELL, 100.0, 2);
        auto trade_vector = book.ProcessOrder(sell1);
        CHECK(trade_vector.empty(), "Resting SELL #1 produces no trade_vector");
    }
    {
        Order sell2(2, 2, "Trump", SIDE::SELL, 100.0, 2);
        auto trade_vector = book.ProcessOrder(sell2);
        CHECK(trade_vector.empty(), "Resting SELL #2 produces no trade_vector");
    }

    {
        Order buy1(3, 3, "Biden", SIDE::BUY, 100.0, 3);
        auto trade_vector = book.ProcessOrder(buy1);

        CHECK(trade_vector.size() == 2, "Incoming BUY produces exactly 2 trade_vector");

        if (trade_vector.size() >= 2) {
            auto t0 = trade_vector[0];
            auto t1 = trade_vector[1];

            CHECK(t0->Buyer() == "Biden", "Trade 0 buyer is Biden");
            CHECK(t0->Seller() == "Donald", "Trade 0 seller is Donald (FIFO)");
            CHECK(t0->Qty() == 2, "Trade 0 qty is 2 (fills Donald)");
            CHECK(t0->Price() == 100.0, "Trade 0 price is resting price 100.0");

            CHECK(t1->Buyer() == "Biden", "Trade 1 buyer is Biden");
            CHECK(t1->Seller() == "Trump", "Trade 1 seller is Trump (second)");
            CHECK(t1->Qty() == 1, "Trade 1 qty is 1 (partial fill)");
            CHECK(t1->Price() == 100.0, "Trade 1 price is resting price 100.0");
        }
    }

    {
        Order buy2(4, 4, "Obama", SIDE::BUY, 101.0, 2);
        auto trade_vector = book.ProcessOrder(buy2);

        CHECK(trade_vector.size() == 1,
              "Second BUY produces exactly 1 trade (consumes remainder)");

        if (!trade_vector.empty()) {
            auto t = trade_vector[0];
            CHECK(t->Buyer() == "Obama", "Remainder trade buyer is Obama");
            CHECK(t->Seller() == "Trump", "Remainder trade seller is Trump");
            CHECK(t->Qty() == 1, "Remainder trade qty is 1");
            CHECK(t->Price() == 100.0,
                  "Remainder trade price is resting price 100.0");
        }
    }
}

static void test_price_priority() {
    std::cout << "\n=== test_price_priority ===\n";

    OrderBook book;

    {
        Order buy1(1, 1, "Biden", SIDE::BUY, 100, 1);
        auto trade_vector = book.ProcessOrder(buy1);
        CHECK(trade_vector.empty(), "Resting BUY #1 produces no trade_vector");
    }
    {
        Order buy2(2, 2, "Obama", SIDE::BUY, 102, 1);
        auto trade_vector = book.ProcessOrder(buy2);
        CHECK(trade_vector.empty(), "Resting BUY #2 produces no trade_vector");
    }
    {
        Order sell1(3, 3, "Donald", SIDE::SELL, 100, 1);
        auto trade_vector = book.ProcessOrder(sell1);
        CHECK(trade_vector.size() == 1, "Incoming SELL produces 1 trade.");

        if (trade_vector.size() == 1) {
            auto t = trade_vector[0];

            CHECK(t->Buyer() == "Obama",
                  "Trade buyer is Obama (Price Priority)");
            CHECK(t->Seller() == "Donald",
                  "Trade buyer is Obama (Price Priority)");
            CHECK(t->Qty() == 1, "Trade qty is 1");
            CHECK(t->Price() == 102.0, "Trade price is resting price 102.0");
        }
    }
}

int main() {
    test_cancel_prevents_match();
    test_fifo_same_price_sell_side();
    test_price_priority();

    if (failures == 0) {
        std::cout << "\nALL TESTS PASSED\n";
        return 0;
    } else {
        std::cerr << "\nTESTS FAILED: " << failures << "\n";
        return 1;
    }
}
