#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "orderbook/Order.h"
#include "orderbook/OrderBook.h"

SIDE ParseSide(char c) {
    if (c == 'B')
        return SIDE::BUY;
    if (c == 'S')
        return SIDE::SELL;
    throw std::runtime_error("Invalid side character");
}

int main() {
    const std::string filename = "data/order_data.csv";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << "\n";
        return 1;
    }

    std::string line;
    bool is_header = true;

    OrderBook book;
    TradeVector trade_vector;

    uint64_t next_order_id = 1;
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        if (is_header) {
            is_header = false; // skip header row
            continue;
        }

        std::stringstream ss(line);
        std::string field;

        int64_t timestamp{};
        std::string trader;
        char side_char{};
        double price{};
        uint32_t qty{};

        // timestamp
        if (!std::getline(ss, field, ','))
            continue;
        timestamp = std::stoll(field);

        // trader
        if (!std::getline(ss, field, ','))
            continue;
        trader = field;

        // side
        if (!std::getline(ss, field, ','))
            continue;
        if (field.size() != 1) {
            std::cerr << "Invalid side value on row: " << line << "\n";
            continue;
        }
        side_char = field[0];

        // price
        if (!std::getline(ss, field, ','))
            continue;
        price = std::stod(field);

        // qty
        if (!std::getline(ss, field, ','))
            continue;
        qty = static_cast<uint32_t>(std::stoul(field));

        // Convert to enum SIDE
        SIDE side;
        try {
            side = ParseSide(side_char);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing side at timestamp " << timestamp << ": "
                      << e.what() << "\n";
            continue;
        }

        // Create Order instance
        Order order(next_order_id++, timestamp, trader, side, price, qty);

        // Pass to matching engine
        const auto new_trade_vector = book.ProcessOrder(order);
        trade_vector.insert(trade_vector.end(), new_trade_vector.begin(), new_trade_vector.end());

        // Testing the CancelOrder
        if (next_order_id == 10) {
            book.CancelOrder(3);
        }
    }

    // Either output trade_vector and book here or set breakpoints to inspect
    return 0;
}
