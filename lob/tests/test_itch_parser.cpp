#include "ITCHParser.h"

#include "Exchange.h"

#include <fstream>
#include <iostream>
#include <vector>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT(cond)                                                          \
  do {                                                                        \
    ++tests_run;                                                              \
    if (cond) {                                                               \
      ++tests_passed;                                                         \
    } else {                                                                  \
      std::cerr << "FAIL: " << #cond << " (" << __FILE__ << ":" << __LINE__  \
                << ")\n";                                                     \
    }                                                                         \
  } while (0)

namespace {

void writeU16BE(std::vector<uint8_t>& out, uint16_t value) {
  out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>(value & 0xFF));
}

std::vector<uint8_t> buildAddOrder(char type, uint64_t order_ref, char side,
                                   uint32_t shares, const std::string& symbol,
                                   uint32_t itch_price) {
  const size_t size = type == 'F' ? 40 : 36;
  std::vector<uint8_t> msg(size, 0);
  msg[0] = static_cast<uint8_t>(type);
  msg[2] = 1;
  msg[4] = 1;
  for (int i = 0; i < 8; ++i) {
    msg[11 + i] = static_cast<uint8_t>((order_ref >> (56 - 8 * i)) & 0xFF);
  }
  msg[19] = static_cast<uint8_t>(side);
  for (int i = 0; i < 4; ++i) {
    msg[20 + i] = static_cast<uint8_t>((shares >> (24 - 8 * i)) & 0xFF);
  }
  std::string padded = symbol;
  padded.resize(8, ' ');
  for (size_t i = 0; i < 8; ++i) {
    msg[24 + i] = static_cast<uint8_t>(padded[i]);
  }
  for (int i = 0; i < 4; ++i) {
    msg[32 + i] = static_cast<uint8_t>((itch_price >> (24 - 8 * i)) & 0xFF);
  }
  return msg;
}

std::vector<uint8_t> buildDelete(uint64_t order_ref) {
  std::vector<uint8_t> msg(19, 0);
  msg[0] = 'D';
  msg[2] = 1;
  msg[4] = 1;
  for (int i = 0; i < 8; ++i) {
    msg[11 + i] = static_cast<uint8_t>((order_ref >> (56 - 8 * i)) & 0xFF);
  }
  return msg;
}

std::vector<uint8_t> buildReplace(uint64_t original_ref, uint64_t new_ref,
                                  uint32_t shares, uint32_t itch_price) {
  std::vector<uint8_t> msg(35, 0);
  msg[0] = 'U';
  msg[2] = 1;
  msg[4] = 1;
  for (int i = 0; i < 8; ++i) {
    msg[11 + i] = static_cast<uint8_t>((original_ref >> (56 - 8 * i)) & 0xFF);
    msg[19 + i] = static_cast<uint8_t>((new_ref >> (56 - 8 * i)) & 0xFF);
  }
  for (int i = 0; i < 4; ++i) {
    msg[27 + i] = static_cast<uint8_t>((shares >> (24 - 8 * i)) & 0xFF);
    msg[31 + i] = static_cast<uint8_t>((itch_price >> (24 - 8 * i)) & 0xFF);
  }
  return msg;
}

void appendLengthPrefixed(std::vector<uint8_t>& file,
                          const std::vector<uint8_t>& message) {
  writeU16BE(file, static_cast<uint16_t>(message.size()));
  file.insert(file.end(), message.begin(), message.end());
}

bool writeFile(const std::string& path, const std::vector<uint8_t>& data) {
  std::ofstream out(path, std::ios::binary);
  if (!out) return false;
  out.write(reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size()));
  return static_cast<bool>(out);
}

}  // namespace

void test_add_orders_cross() {
  Exchange exchange;
  ITCHParser parser(exchange);

  ASSERT(parser.processMessage(buildAddOrder('A', 1, 'S', 100, "AAPL", 1000000).data(), 36));
  ASSERT(parser.processMessage(buildAddOrder('A', 2, 'B', 50, "AAPL", 1000000).data(), 36));

  ASSERT(exchange.getOrder("AAPL", 1)->quantity == 50);
  ASSERT(!exchange.getOrder("AAPL", 2).has_value());
}

void test_delete_order() {
  Exchange exchange;
  ITCHParser parser(exchange);

  ASSERT(parser.processMessage(buildAddOrder('A', 10, 'B', 100, "MSFT", 2000000).data(), 36));
  ASSERT(parser.processMessage(buildDelete(10).data(), 19));

  ASSERT(!exchange.getOrder("MSFT", 10).has_value());
}

void test_replace_order() {
  Exchange exchange;
  ITCHParser parser(exchange);

  ASSERT(parser.processMessage(buildAddOrder('A', 20, 'S', 100, "GOOG", 1500000).data(), 36));
  ASSERT(parser.processMessage(buildReplace(20, 21, 40, 1510000).data(), 35));

  ASSERT(!exchange.getOrder("GOOG", 20).has_value());
  ASSERT(exchange.getOrder("GOOG", 21)->quantity == 40);
  ASSERT(exchange.getOrder("GOOG", 21)->price == 15100);
}

void test_mpid_add_order() {
  Exchange exchange;
  ITCHParser parser(exchange);

  ASSERT(parser.processMessage(buildAddOrder('F', 30, 'B', 25, "TSLA", 2500000).data(), 40));
  ASSERT(exchange.getOrder("TSLA", 30)->quantity == 25);
}

void test_replay_length_prefixed_file() {
  std::vector<uint8_t> file;
  appendLengthPrefixed(file, buildAddOrder('A', 40, 'B', 100, "AAPL", 1000000));
  appendLengthPrefixed(file, buildDelete(40));

  const std::string path = "data/sample_itch.bin";
  ASSERT(writeFile(path, file));

  Exchange exchange;
  ITCHParser parser(exchange);
  const ITCHParseStats stats = parser.replayFile(path);

  ASSERT(stats.messages == 2);
  ASSERT(stats.add_orders == 1);
  ASSERT(stats.deletes == 1);
  ASSERT(stats.errors == 0);
  ASSERT(!exchange.getOrder("AAPL", 40).has_value());
}

int main() {
  test_add_orders_cross();
  test_delete_order();
  test_replace_order();
  test_mpid_add_order();
  test_replay_length_prefixed_file();

  std::cout << tests_passed << "/" << tests_run << " tests passed\n";
  return tests_passed == tests_run ? 0 : 1;
}
