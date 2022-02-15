#include "gtest/gtest.h"
#include "jflect/parser.hpp"

#include <array>
#include <utility>
#include <string_view>

using namespace std::string_view_literals;

// clang-format off
constexpr std::string_view trim(std::string_view sv) noexcept { jflect::parser::trim(sv); return sv; }
constexpr std::string_view read(std::string_view sv, char c) noexcept { jflect::parser::read(sv, c); return sv; }
constexpr std::string_view trim_read(std::string_view sv, char c) noexcept { jflect::parser::trim_read(sv, c); return sv; }
constexpr std::string_view trim_read_trim(std::string_view sv, char c) noexcept { jflect::parser::trim_read_trim(sv, c); return sv; }
constexpr std::pair<std::string_view, bool> optional_read(std::string_view sv, char c) noexcept { const auto b = jflect::parser::optional_read(sv, c); return {sv, b}; }
template<class P> constexpr std::pair<std::string_view, bool> optional_read_if(std::string_view sv, P p) noexcept { const auto b = jflect::parser::optional_read_if(sv, p); return {sv, b}; }
// clang-format on

TEST(json_parser, trim) {
  const auto sv1 = "hello world"sv;
  const auto sv2 = " \r hello world "sv;
  const auto sv3 = "\n \t hello world"sv;

  ASSERT_EQ(trim(sv1), sv1.substr(0));
  ASSERT_EQ(trim(sv2), sv2.substr(3));
  ASSERT_EQ(trim(sv3), sv3.substr(4));
}

TEST(json_parser, read) {
  const auto sv1 = "abc"sv;
  const auto sv2 = "cba"sv;
  [[maybe_unused]] const auto sv3 = ""sv;

  ASSERT_EQ(read(sv1, 'a'), sv1.substr(1));
  ASSERT_EQ(read(sv2, 'c'), sv2.substr(1));
#ifndef NDEBUG
  EXPECT_EXIT(read(sv2, 'a'), testing::KilledBySignal(SIGABRT), "");
  EXPECT_EXIT(read(sv3, ' '), testing::KilledBySignal(SIGABRT), "");
#endif
}

TEST(json_parser, trim_read) {
  const auto sv1 = "\t\thello trim!"sv;
  const auto sv2 = "\t\n [] "sv;
  const auto sv3 = "?"sv;
  [[maybe_unused]] const auto sv4 = "   "sv;

  ASSERT_EQ(trim_read(sv1, 'h'), sv1.substr(3));
  ASSERT_EQ(trim_read(sv2, '['), sv2.substr(4));
  ASSERT_EQ(trim_read(sv3, '?'), sv3.substr(1));
#ifndef NDEBUG
  EXPECT_EXIT(trim_read(sv2, ']'), testing::KilledBySignal(SIGABRT), "");
  EXPECT_EXIT(trim_read(sv4, ' '), testing::KilledBySignal(SIGABRT), "");
#endif
}

TEST(json_parser, trim_read_trim) {
  const auto sv1 = "\r\r: \t 1.0"sv;
  const auto sv2 = "\t\n,"sv;
  const auto sv3 = "0"sv;
  [[maybe_unused]] const auto sv4 = "\t \t"sv;

  ASSERT_EQ(trim_read_trim(sv1, ':'), sv1.substr(6));
  ASSERT_EQ(trim_read_trim(sv2, ','), sv2.substr(3));
  ASSERT_EQ(trim_read_trim(sv3, '0'), sv3.substr(1));
#ifndef NDEBUG
  EXPECT_EXIT(trim_read_trim(sv2, ':'), testing::KilledBySignal(SIGABRT), "");
  EXPECT_EXIT(trim_read_trim(sv3, '1'), testing::KilledBySignal(SIGABRT), "");
  EXPECT_EXIT(trim_read_trim(sv4, ' '), testing::KilledBySignal(SIGABRT), "");
#endif
}

TEST(json_parser, optional_read) {
  const auto sv1 = "G"sv;
  const auto sv2 = "[]\n"sv;
  const auto sv3 = "  -0.5e-10"sv;
  const auto sv4 = ""sv;

  ASSERT_EQ(optional_read(sv1, 'G'), std::make_pair(sv1.substr(1), true));
  ASSERT_EQ(optional_read(sv1, 'g'), std::make_pair(sv1, false));
  ASSERT_EQ(optional_read(sv2, '['), std::make_pair(sv2.substr(1), true));
  ASSERT_EQ(optional_read(sv2, ']'), std::make_pair(sv2, false));
  ASSERT_EQ(optional_read(sv3, '-'), std::make_pair(sv3, false));
  ASSERT_EQ(optional_read(sv4, ' '), std::make_pair(sv4, false));
}

TEST(json_parser, optional_read_if) {
  const auto sv1 = "abcdef"sv;
  const auto sv2 = "{}"sv;
  const auto sv3 = "-0.5e-10"sv;
  const auto sv4 = ""sv;

  // clang-format off
  ASSERT_EQ(optional_read_if(sv1, isalpha), std::make_pair(sv1.substr(1), true));
  ASSERT_EQ(optional_read_if(sv1, isdigit), std::make_pair(sv1, false));
  ASSERT_EQ(optional_read_if(sv2, std::not_fn(iscntrl)), std::make_pair(sv2.substr(1), true));
  ASSERT_EQ(optional_read_if(sv2, [](auto) { return false; }), std::make_pair(sv2, false));
  ASSERT_EQ(optional_read_if(sv3, [](char c) { return c == '-' || c == '+'; }), std::make_pair(sv3.substr(1), true));
  ASSERT_EQ(optional_read_if(sv4, [](auto) { return true; }), std::make_pair(sv4, false));
  // clang-format on
}

TEST(json_parser, read_value) {
  using T = std::pair<std::string_view, std::size_t>;
  const auto tests = std::array{
      // T{"", 0}, // [NOTE] should this be valid?
      T{"\ttrue", 5},
      T{"\nfalse ", 6},
      T{"null, null", 4},
      T{"\"simple string\" ", 15},
      T{R"("string \t with \n escape characters\n\r", [3, 2])", 41},
      T{R"("string with \uaF09 unicode", 99999.32)", 28},
      T{"42", 2},
      T{"17.3", 4},
      T{"-1", 2},
      T{"13.8e10", 7},
      T{"-7432E-4", 8},
      T{"1e1", 3},
      T{"1E0", 3},
      T{"[]", 2},
      T{"[\"hello range\"]", 15},
      T{"[ 13e-10, 42.2 , \"not empty \" ] ", 31},
      T{"{  }", 4},
      T{R"({ "alpha": 2.3, "beta": 50.0 })", 30},
  };

  for (const auto& [sv, pos] : tests) {
    const auto result = jflect::parser::read_value(sv);
    ASSERT_EQ(result, sv.substr(pos));
  }
}
