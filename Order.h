#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

enum class SIDE { BUY, SELL };

class Order {
  public:
    Order(const uint64_t _order_id, const int64_t _timestamp,
          std::string _trader, const SIDE _side, const double _price,
          const uint32_t _qty)
        : order_id{_order_id}, timestamp{_timestamp},
          trader{std::move(_trader)}, side{_side}, price{_price}, qty{_qty},
          qty_remaining{_qty} {}

    [[nodiscard]] uint64_t OrderId() const { return order_id; }
    [[nodiscard]] int64_t Timestamp() const { return timestamp; }
    [[nodiscard]] const std::string &Trader() const { return trader; }
    [[nodiscard]] SIDE Side() const { return side; }
    [[nodiscard]] double Price() const { return price; }
    [[nodiscard]] uint32_t Qty() const { return qty; }
    [[nodiscard]] uint32_t QtyRemaining() const { return qty_remaining; }
    void QtyRemaining(uint32_t _qty_remaining) {
        qty_remaining = _qty_remaining;
    }

  private:
    int64_t timestamp;
    std::string trader;
    SIDE side;
    double price;
    uint32_t qty;
    uint32_t qty_remaining;
};
using OrderPtr = std::shared_ptr<Order>;
