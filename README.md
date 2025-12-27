# C++ Limit Order Book

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

A high-performance, low-latency Limit Order Book (LOB) and Matching Engine implementation in modern C++. Designed to handle standard order types, execution reporting, and real-time market data structure management.

## Overview

This project implements a standard price-time priority matching engine. It supports:
* **Order Management**: Add, Cancel, and Modify orders.
* **Matching Logic**: Continuous matching for Market and Limit orders.
* **Data Structures**: Efficient use of maps and pointers for O(1) lookups and fast level updates.
* **Architecture**: Separation of concerns between the API (`OrderBook`), Data (`Order`, `Trade`), and Execution (`MatchResult`).

## Project Structure

The codebase follows a clean separation of interface and implementation:

```text
order_book_cpp/
├── app/                  # Main application entry point (Driver)
├── include/orderbook/    # Public Header files
│   ├── OrderBook.h       # Core engine interface
│   ├── Order.h           # Order definition
│   └── Trade.h           # Trade execution definition
├── data/                 # Source of data
├── src/                  # Source implementations
├── tests/                # GoogleTest unit test suite
├── CMakeLists.txt        # Build configuration
└── compile_commands.json # Clang compilation database
```


## Build Instructions
This project uses CMake. Ensure you have CMake (3.15+) and a C++ compiler (GCC/Clang/MSVC) installed.

### 1. Clone and Configure 
```bash
git clone [https://github.com/lanteignel93/order_book_cpp.git](https://github.com/lanteignel93/order_book_cpp.git)
cd order_book_cpp

# Create build directory
mkdir build && cd build

# Configure the project
cmake ..
```

### 2. Compile 
```bash
# Build the application and library
cmake --build .
```

### 3. Run Application 
The main trading application executable is named `orderbook_app`. 
```bash
./orderbook_app
```

### 4. Testing
This project uses `enable_testing()` for unit tests. You can run them via CTest or by executing the test binary directly.
```bash
cd build

ctest --output-on-failure

./orderbook_tests
```

## Key Components
**Order Types**

* Limit: Buy/Sell at a specific price or better.
* Market: Immediate execution at best available price.
* Fill-and-Kill (FaK): Execute what is possible immediately; cancel the rest.

**Data Structures**

* OrderBookLevel: Represents a price level containing a queue of orders.
* Order: Contains OrderId, Side, Price, and Quantity.
* MatchResult: Struct returning fills and partial fills from an operation.

## Contributing

* Fork the repository
* Create your feature branch (git checkout -b feature/NewOrderType)
* Commit your changes (git commit -m 'Add StopLoss orders')
* Push to the branch (git push origin feature/NewOrderType)
* Open a Pull Request

## License

Distributed under the MIT License. See LICENSE for more information.
