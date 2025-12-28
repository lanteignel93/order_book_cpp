cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(POLICY CMP0156)
  cmake_policy(SET CMP0156 NEW)
endif()

project(orderbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------ Library ------------------
add_library(orderbook_lib
  src/OrderBook.cpp
)

target_include_directories(orderbook_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)

if(TARGET Arrow::arrow_shared)
  set(ARROW_TARGET Arrow::arrow_shared)
elseif(TARGET Arrow::arrow_static)
  set(ARROW_TARGET Arrow::arrow_static)
elseif(TARGET Arrow::arrow)
  set(ARROW_TARGET Arrow::arrow)
else()
  message(FATAL_ERROR
    "Arrow was found, but no known Arrow CMake target exists "
    "(Arrow::arrow_shared / Arrow::arrow_static / Arrow::arrow)."
  )
endif()

if(TARGET Parquet::parquet_shared)
  set(PARQUET_TARGET Parquet::parquet_shared)
elseif(TARGET Parquet::parquet_static)
  set(PARQUET_TARGET Parquet::parquet_static)
elseif(TARGET Parquet::parquet)
  set(PARQUET_TARGET Parquet::parquet)
else()
  message(FATAL_ERROR
    "Parquet was found, but no known Parquet CMake target exists "
    "(Parquet::parquet_shared / Parquet::parquet_static / Parquet::parquet)."
  )
endif()

add_executable(orderbook_app
  app/main.cpp
)

target_link_libraries(orderbook_app PRIVATE
  orderbook_lib
  ${ARROW_TARGET}
  ${PARQUET_TARGET}
)

enable_testing()

add_executable(orderbook_tests
  tests/orderbook_tests.cpp
)

target_link_libraries(orderbook_tests PRIVATE orderbook_lib)

add_test(NAME orderbook_tests COMMAND orderbook_tests)
