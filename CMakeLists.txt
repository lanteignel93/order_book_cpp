cmake_minimum_required(VERSION 3.20)
project(orderbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(orderbook_lib
  src/OrderBook.cpp
)

target_include_directories(orderbook_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(orderbook_app
  app/main.cpp
)

target_link_libraries(orderbook_app PRIVATE orderbook_lib)

enable_testing() 

add_executable(orderbook_tests
  tests/orderbook_tests.cpp
)

target_link_libraries(orderbook_tests PRIVATE orderbook_lib)

add_test(NAME orderbook_tests COMMAND orderbook_tests)
