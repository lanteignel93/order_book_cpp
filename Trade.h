#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Trade
{
public:
  Trade(int64_t _timestamp, std::string _buyer, std::string _seller,
        const double _price, const uint32_t _qty)
   : timestamp{_timestamp}, buyer{std::move(_buyer)}, seller{std::move(_seller)},
      price{_price}, qty{_qty}
  {
  }

  [[nodiscard]] int64_t Timestamp() const {return timestamp;}
  [[nodiscard]] const std::string& Buyer() const {return buyer;}
  [[nodiscard]] const std::string& Seller() const {return seller;}
  [[nodiscard]] double Price() const {return price;}
  [[nodiscard]] uint32_t Qty() const {return qty;}

private:
  int64_t timestamp;
  std::string buyer;
  std::string seller;
  double price;
  uint32_t qty;
};
using TradePtr = std::shared_ptr<Trade>;
using TradeVector = std::vector<TradePtr>;