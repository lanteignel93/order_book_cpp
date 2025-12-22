cmake_minimum_required(VERSION 3.30)
project(book_test)

set(CMAKE_CXX_STANDARD 20)

add_executable(book_test main.cpp
        OrderBook.cpp
        OrderBook.h
        Order.h
        Trade.h)
