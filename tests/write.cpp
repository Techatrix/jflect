#include "gtest/gtest.h"
#include "jflect/jflect.hpp"

#include <set>
#include <map>
#include <optional>

TEST(json_write, boolean) {
  ASSERT_EQ(jflect::write(true), "true");
  ASSERT_EQ(jflect::write(false), "false");
}

TEST(json_write, integral) {
  ASSERT_EQ(jflect::write(1), "1");
  ASSERT_EQ(jflect::write(24u), "24");
  ASSERT_EQ(jflect::write(-30l), "-30");
}

TEST(json_write, floating_point) {
  ASSERT_EQ(jflect::write(1.0), "1.000000");
  ASSERT_EQ(jflect::write(-5.3f), "-5.300000");
  ASSERT_EQ(jflect::write(3.14159f), "3.141590");
  ASSERT_EQ(jflect::write(-48.32), "-48.320000");
}

TEST(json_write, enumerator) {
  enum weekdays { monday, tuesday, wednesday, thursday, friday, saturday, sunday };
  using enum weekdays;

  ASSERT_EQ(jflect::write(monday), "\"monday\"");
  ASSERT_EQ(jflect::write(thursday), "\"thursday\"");
  ASSERT_EQ(jflect::write(sunday), "\"sunday\"");
  ASSERT_EQ(jflect::write(static_cast<weekdays>(99)), "\"\"");
}

TEST(json_write, enumerator_unordered) {
  enum unordered_weekdays {
    wednesday = 2,
    thursday = 3,
    saturday = 5,
    friday = 4,
    tuesday = 1,
    monday = 0,
    sunday = 6
  };
  using enum unordered_weekdays;

  ASSERT_EQ(jflect::write(wednesday), "\"wednesday\"");
  ASSERT_EQ(jflect::write(friday), "\"friday\"");
  ASSERT_EQ(jflect::write(sunday), "\"sunday\"");
  ASSERT_EQ(jflect::write(unordered_weekdays(-1)), "\"\"");
}

TEST(json_write, enumerator_uncontiguous) {
  enum uncontiguous_weekdays { tuesday = 1, wednesday = 2, saturday = 5 };
  using enum uncontiguous_weekdays;

  ASSERT_EQ(jflect::write(tuesday), "\"tuesday\"");
  ASSERT_EQ(jflect::write(wednesday), "\"wednesday\"");
  ASSERT_EQ(jflect::write(saturday), "\"saturday\"");
  ASSERT_EQ(jflect::write(uncontiguous_weekdays(0)), "\"\"");
  ASSERT_EQ(jflect::write(uncontiguous_weekdays(4)), "\"\"");
  ASSERT_EQ(jflect::write(uncontiguous_weekdays(6)), "\"\"");
}

TEST(json_write, enumerator_empty) {
  enum empty_enum {};

  ASSERT_EQ(jflect::write(empty_enum()), "\"\"");
  ASSERT_EQ(jflect::write(empty_enum(0)), "\"\"");
  ASSERT_EQ(jflect::write(empty_enum(1)), "\"\"");
}

TEST(json_write, string) {
  ASSERT_EQ(jflect::write(""), "\"\"");
  ASSERT_EQ(jflect::write("hello world"), "\"hello world\"");
  ASSERT_EQ(jflect::write("this is a c-style string"), "\"this is a c-style string\"");
  ASSERT_EQ(jflect::write(std::string("this is a std::string")), "\"this is a std::string\"");
  ASSERT_EQ(jflect::write(std::string_view("this is a std::string_view")), "\"this is a std::string_view\"");

  const auto str = std::string("this is a const std::string");
  ASSERT_EQ(jflect::write(str), "\"this is a const std::string\"");
}

TEST(json_write, range) {
  ASSERT_EQ(jflect::write(std::initializer_list<int>{}), "[]");
  ASSERT_EQ(jflect::write(std::array{1, 2, 3}), "[1,2,3]");

  const auto set = std::set<std::string>({"this is a string", "this is another string"});
  ASSERT_EQ(jflect::write(set), "[\"this is a string\",\"this is another string\"]");

  const std::vector<std::vector<int>> nestedRange{{1, 2}, {3, 4}};
  ASSERT_EQ(jflect::write(nestedRange), "[[1,2],[3,4]]");
}

TEST(json_write, map) {
  const std::map<std::string, int> map1{{"first", 42}, {"second", 36}};
  const std::map<std::string, std::pair<unsigned long, int>> map2{{"alpha", {33, -11}}, {"beta", {88, -1}}};
  const std::map<std::string, std::array<std::string, 3>> map3{{"begin", {"a", "b", "c"}}, {"end", {"x", "y", "z"}}};

  ASSERT_EQ(jflect::write(map1), "{\"first\":42,\"second\":36}");
  ASSERT_EQ(jflect::write(map2), "{\"alpha\":[33,-11],\"beta\":[88,-1]}");
  ASSERT_EQ(jflect::write(map3), "{\"begin\":[\"a\",\"b\",\"c\"],\"end\":[\"x\",\"y\",\"z\"]}");
}

TEST(json_write, structure) {
  struct T1 {
    int alpha;
    long long beta;
  };

  struct T2 {
    std::string question;
    int answer;
  };

  enum E { e_choice1, e_choice2, e_choice3 };
  struct T3 {
    T1 first;
    E second;
  };

  struct T4 {
    std::vector<std::string> collection;
    std::set<E> enums;
  };

  static_assert(jflect::cpt::public_struct<T1>);
  static_assert(jflect::cpt::public_struct<T2>);
  static_assert(jflect::cpt::public_struct<T3>);
  static_assert(jflect::cpt::public_struct<T4>);

  ASSERT_EQ(jflect::write(T1{.alpha = 543, .beta = -1234}), "{\"alpha\":543,\"beta\":-1234}");

  ASSERT_EQ(jflect::write(T2{.question = "What is the answer to everything?", .answer = 42}),
            "{\"question\":\"What is the answer to everything?\",\"answer\":42}");

  ASSERT_EQ(jflect::write(T3{.first = {16, 80}, .second = e_choice2}),
            "{\"first\":{\"alpha\":16,\"beta\":80},\"second\":\"e_choice2\"}");

  ASSERT_EQ(jflect::write(T4{.collection = {"roses are red", "violets are blue", "there is a missing }", "on line 32"},
                             .enums = {e_choice1, e_choice3}}),
            "{\"collection\":[\"roses are red\",\"violets are blue\",\"there is a missing }\",\"on line 32\"],\""
            "enums\":[\"e_choice1\",\"e_choice3\"]}");

}

TEST(json_write, pair) {
  ASSERT_EQ(jflect::write(std::make_pair(1, 3)), "[1,3]");
  ASSERT_EQ(jflect::write(std::make_pair(1.5, 3)), "[1.500000,3]");
  ASSERT_EQ(jflect::write(std::make_pair("hello", "world")), "[\"hello\",\"world\"]");
}

TEST(json_write, tuple) {
  ASSERT_EQ(jflect::write(std::make_tuple()), "[]");
  ASSERT_EQ(jflect::write(std::make_tuple(1, 3, 4)), "[1,3,4]");
  ASSERT_EQ(jflect::write(std::make_tuple(3.3, -4, "this is a c-style string")),
            "[3.300000,-4,\"this is a c-style string\"]");
  ASSERT_EQ(jflect::write(std::make_tuple("hello", "beautiful", "world")), "[\"hello\",\"beautiful\",\"world\"]");
}

/*------------------------------ standard types ------------------------------*/

TEST(json_write, optional) {
  const auto opt = std::make_optional(33);
  ASSERT_EQ(jflect::write(opt), "33");
  ASSERT_EQ(jflect::write(std::make_optional(5)), "5");
  ASSERT_EQ(jflect::write(std::optional<float>(std::nullopt)), "null");
  ASSERT_EQ(jflect::write(std::make_optional("null")), "\"null\"");
}
