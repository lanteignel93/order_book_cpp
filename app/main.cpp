#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include "orderbook/Order.h"
#include "orderbook/OrderBook.h"

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>

static SIDE ParseSideString(const std::string &s) {
    if (s == "B" || s == "BUY")
        return SIDE::BUY;
    if (s == "S" || s == "SELL")
        return SIDE::SELL;
    throw std::runtime_error("Invalid side value: " + s);
}

static TYPE ParseTypeString(const std::string &s) {
    if (s == "LO" || s == "LIMIT_ORDER")
        return TYPE::LIMIT_ORDER;
    throw std::runtime_error("Invalid type value: " + s);
}

static int GetColumnIndex(const std::shared_ptr<arrow::Schema> &schema,
                          const std::string &name) {
    const int idx = schema->GetFieldIndex(name);
    if (idx == -1) {
        throw std::runtime_error("Missing required column: " + name);
    }
    return idx;
}

static std::string GetStringValue(const std::shared_ptr<arrow::Array> &arr,
                                  int64_t i) {
    if (!arr || arr->IsNull(i))
        return {};

    switch (arr->type_id()) {
    case arrow::Type::STRING: {
        auto a = std::static_pointer_cast<arrow::StringArray>(arr);
        return a->GetString(i);
    }
    case arrow::Type::LARGE_STRING: {
        auto a = std::static_pointer_cast<arrow::LargeStringArray>(arr);
        return a->GetString(i);
    }
    case arrow::Type::DICTIONARY: {
        auto dict = std::static_pointer_cast<arrow::DictionaryArray>(arr);
        const auto &dict_values = dict->dictionary();

        // dictionary values should be strings
        if (dict_values->type_id() == arrow::Type::STRING) {
            auto dv = std::static_pointer_cast<arrow::StringArray>(dict_values);

            // indices can be int8/int16/int32/int64 depending on encoding
            const auto &idx = dict->indices();
            switch (idx->type_id()) {
            case arrow::Type::INT8: {
                auto ia = std::static_pointer_cast<arrow::Int8Array>(idx);
                return dv->GetString(ia->Value(i));
            }
            case arrow::Type::INT16: {
                auto ia = std::static_pointer_cast<arrow::Int16Array>(idx);
                return dv->GetString(ia->Value(i));
            }
            case arrow::Type::INT32: {
                auto ia = std::static_pointer_cast<arrow::Int32Array>(idx);
                return dv->GetString(ia->Value(i));
            }
            case arrow::Type::INT64: {
                auto ia = std::static_pointer_cast<arrow::Int64Array>(idx);
                return dv->GetString(ia->Value(i));
            }
            default:
                throw std::runtime_error("Unsupported dictionary index type: " +
                                         idx->type()->ToString());
            }
        }

        // Some writers can produce large_string dictionaries
        if (dict_values->type_id() == arrow::Type::LARGE_STRING) {
            auto dv =
                std::static_pointer_cast<arrow::LargeStringArray>(dict_values);
            const auto &idx = dict->indices();
            if (idx->type_id() == arrow::Type::INT32) {
                auto ia = std::static_pointer_cast<arrow::Int32Array>(idx);
                return dv->GetString(ia->Value(i));
            }
            throw std::runtime_error(
                "Unsupported large_string dictionary index type: " +
                idx->type()->ToString());
        }

        throw std::runtime_error("Dictionary values are not string type: " +
                                 dict_values->type()->ToString());
    }
    default:
        throw std::runtime_error("Column is not a string-like type: " +
                                 arr->type()->ToString());
    }
}

int main() {
    auto start = std::chrono::system_clock::now();
    const std::string filename = "data/order_data.parquet";

    auto infile_result = arrow::io::ReadableFile::Open(filename);
    if (!infile_result.ok()) {
        std::cerr << "Failed to open parquet file: " << filename
                  << " error=" << infile_result.status().ToString() << "\n";
        return 1;
    }
    std::shared_ptr<arrow::io::ReadableFile> infile = *infile_result;

    auto reader_result =
        parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
    if (!reader_result.ok()) {
        std::cerr << "Failed to create parquet reader: "
                  << reader_result.status().ToString() << "\n";
        return 1;
    }
    std::unique_ptr<parquet::arrow::FileReader> reader =
        std::move(reader_result).ValueUnsafe();

    auto rb_reader_result = reader->GetRecordBatchReader();
    if (!rb_reader_result.ok()) {
        std::cerr << "Failed to get RecordBatchReader: "
                  << rb_reader_result.status().ToString() << "\n";
        return 1;
    }
    std::unique_ptr<arrow::RecordBatchReader> rb_reader =
        std::move(rb_reader_result).ValueUnsafe();

    OrderBook book;
    TradeVector trade_vector;
    trade_vector.reserve(1024);

    uint64_t order_id = 1;
    while (true) {
        auto rb_result = rb_reader->Next();
        if (!rb_result.ok()) {
            std::cerr << "Error reading RecordBatch: "
                      << rb_result.status().ToString() << "\n";
            return 1;
        }
        std::shared_ptr<arrow::RecordBatch> batch = *rb_result;
        if (!batch)
            break; // EOF

        auto schema = batch->schema();

        auto now = std::chrono::system_clock::now();
        auto duration_since_start = now - start;
        const int64_t timestamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                duration_since_start)
                .count();
        const int col_trader = GetColumnIndex(schema, "trader");
        const int col_side = GetColumnIndex(schema, "side");
        const int col_price = GetColumnIndex(schema, "price");
        const int col_qty = GetColumnIndex(schema, "size");
        const int col_type = GetColumnIndex(schema, "type");

        // Cast columns to expected Arrow array types
        auto trader_arr = std::static_pointer_cast<arrow::StringArray>(
            batch->column(col_trader));
        auto side_arr = std::static_pointer_cast<arrow::StringArray>(
            batch->column(col_side));
        auto price_arr = std::static_pointer_cast<arrow::DoubleArray>(
            batch->column(col_price));
        auto qty_col = batch->column(col_qty);
        if (qty_col->type_id() != arrow::Type::INT64) {
            throw std::runtime_error("Expected 'size' to be int64, got: " +
                                     qty_col->type()->ToString());
        }
        auto qty_arr = std::static_pointer_cast<arrow::Int64Array>(qty_col);
        auto type_arr = std::static_pointer_cast<arrow::StringArray>(
            batch->column(col_type));

        const int64_t n = batch->num_rows();
        for (int64_t i = 0; i < n; ++i) {
            if (trader_arr->IsNull(i) || side_arr->IsNull(i) ||
                price_arr->IsNull(i) || qty_arr->IsNull(i) ||
                type_arr->IsNull(i)) {
                continue;
            }

            const std::string trader = GetStringValue(trader_arr, i);
            const std::string side_s = GetStringValue(side_arr, i);
            const SIDE side = ParseSideString(side_s);
            const double price = price_arr->Value(i);
            const int64_t qty64 = qty_arr->Value(i);
            if (qty64 < 0 || qty64 > std::numeric_limits<uint32_t>::max()) {
                throw std::runtime_error("Invalid qty out of range: " +
                                         std::to_string(qty64));
            }
            const uint32_t qty = static_cast<uint32_t>(qty64);
            const std::string type_s = GetStringValue(type_arr, i);
            const TYPE type = ParseTypeString(type_s);

            Order o(order_id, timestamp, trader, side, price, qty);

            auto trades = book.ProcessOrder(o);
            trade_vector.insert(trade_vector.end(), trades.begin(),
                                trades.end());
            order_id++;
        }
    }

    std::cout << "Processed parquet file: " << filename << "\n";
    std::cout << "Total trades: " << trade_vector.size() << "\n";
    return 0;
}
