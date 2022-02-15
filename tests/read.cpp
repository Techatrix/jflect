#include "gtest/gtest.h"
#include "jflect/jflect.hpp"

TEST(json_read, boolean) {
  ASSERT_EQ(jflect::read<bool>("true"), true);
  ASSERT_EQ(jflect::read<bool>("  false "), false);
}

TEST(json_read, integral) {
  ASSERT_EQ(jflect::read<int>("1"), 1);
  ASSERT_EQ(jflect::read<int>("-25"), -25);
  ASSERT_EQ(jflect::read<unsigned int>(" 36"), 36u);
  ASSERT_EQ(jflect::read<long>(" 100 "), 100l);
}

TEST(json_read, floating_point) {
  ASSERT_FLOAT_EQ(jflect::read<float>("  1.0"), 1.0f);
  ASSERT_DOUBLE_EQ(jflect::read<double>("-5.3f"), -5.3);
  ASSERT_FLOAT_EQ(jflect::read<float>("3.14159  "), 3.141590f);
  ASSERT_DOUBLE_EQ(jflect::read<double>(" -48.32"), -48.320000);
}

TEST(json_read, enumerator) {
  enum weekdays { monday, tuesday, wednesday, thursday, friday, saturday, sunday };
  using enum weekdays;

  ASSERT_EQ(jflect::read<weekdays>("\"monday\""), weekdays::monday);
  ASSERT_EQ(jflect::read<weekdays>("  \"thursday\""), weekdays::thursday);
  ASSERT_EQ(jflect::read<weekdays>(" \"sunday\"  "), weekdays::sunday);
  // ASSERT_EQ(jflect::read<weekdays>("  october"), "");
}

TEST(json_read, string) {
  ASSERT_EQ(jflect::read<std::string>("\"hello world\""), "hello world");
  ASSERT_EQ(jflect::read<std::string>("  \"this is a c-style string\"   "), "this is a c-style string");
}

TEST(json_read, range) {
  ASSERT_TRUE(jflect::read<std::vector<int>>("[]").empty());
  ASSERT_EQ(jflect::read<std::vector<int>>("[1,2,3]"), std::vector({1, 2, 3}));

  const auto set = std::set<std::string>({"this is a string", "this is another string"});
  ASSERT_EQ(jflect::read<std::set<std::string>>("[\"this is a string\",\"this is another string\"]"), set);

  const std::vector<std::vector<int>> nestedRange{{1, 2}, {3, 4}};
  ASSERT_EQ(jflect::read<std::vector<std::vector<int>>>("[[1,2],[3,4]]"), nestedRange);
}

TEST(json_read, map) {
  using M1 = std::map<std::string, int>;
  using M2 = std::map<std::string, std::pair<unsigned long, int>>;
  using M3 = std::map<std::string, std::array<std::string, 3>>;

  const M1 map1{{"first", 42}, {"second", 36}};
  const M2 map2{{"alpha", {33, -11}}, {"beta", {88, -1}}};
  const M3 map3{{"begin", {"a", "b", "c"}}, {"end", {"x", "y", "z"}}};

  ASSERT_EQ(jflect::read<M1>("{\"first\":42,\"second\":36}"), map1);
  ASSERT_EQ(jflect::read<M2>("{\"alpha\":[33,-11],\"beta\":[88,-1]}"), map2);
  ASSERT_EQ(jflect::read<M3>("{\"begin\":[\"a\",\"b\",\"c\"],\"end\":[\"x\",\"y\",\"z\"]}"), map3);
}

TEST(json_read, structure) {
  struct T1 {
    int alpha;
    long long beta;
    bool operator==(const T1& other) const = default;
  };

  struct T2 {
    std::string question;
    int answer;
    bool operator==(const T2& other) const = default;
  };

  enum E { e_choice1, e_choice2, e_choice3 };
  struct T3 {
    T1 first;
    E second;
    bool operator==(const T3& other) const = default;
  };

  struct T4 {
    std::vector<std::string> collection;
    std::set<E> enums;
    bool operator==(const T4& other) const = default;
  };

  static_assert(jflect::cpt::public_struct<T1>);
  static_assert(jflect::cpt::public_struct<T2>);
  static_assert(jflect::cpt::public_struct<T3>);
  static_assert(jflect::cpt::public_struct<T4>);

  const auto t1 = T1{.alpha = 543, .beta = -1234};
  ASSERT_EQ(jflect::read<T1>("{\"alpha\":543,\"beta\":-1234}"), t1);

  const auto t2 = T2{.question = "What is the answer to everything?", .answer = 42};
  ASSERT_EQ(jflect::read<T2>("{\"question\":\"What is the answer to everything?\",\"answer\":42}"), t2);

  const auto t3 = T3{.first = {16, 80}, .second = e_choice2};
  ASSERT_EQ(jflect::read<T3>("{\"first\":{\"alpha\":16,\"beta\":80},\"second\":\"e_choice2\"}"), t3);

  const auto t4 = T4{.collection = {"roses are red", "violets are blue", "there is a missing }", "on line 32"},
                     .enums = {e_choice1, e_choice3}};
  ASSERT_EQ(jflect::read<T4>(
                "{\"collection\":[\"roses are red\",\"violets are blue\",\"there is a missing }\",\"on line 32\"],\""
                "enums\":[\"e_choice1\",\"e_choice3\"]}"),
            t4);
}

TEST(json_read, pair) {
  using T1 = std::pair<int, int>;
  using T2 = std::pair<double, int>;
  using T3 = std::pair<std::string, std::string>;
  ASSERT_EQ(jflect::read<T1>("[1,3]"), T1(1, 3));
  ASSERT_EQ(jflect::read<T2>("[1.500000,3]"), T2(1.5, 3));
  ASSERT_EQ(jflect::read<T3>("[\"hello\",\"world\"]"), T3("hello", "world"));
}

TEST(json_read, tuple) {
  using T1 = std::tuple<>;
  using T2 = std::tuple<int, int, int>;
  using T3 = std::tuple<double, int, std::string>;
  using T4 = std::tuple<std::string, std::string, std::string>;
  ASSERT_EQ(jflect::read<T1>("[]"), T1());
  ASSERT_EQ(jflect::read<T2>("[1,3,4]"), T2(1, 3, 4));
  ASSERT_EQ(jflect::read<T3>("[3.300000,-4,\"this is a c-style string\"]"), T3(3.3, -4, "this is a c-style string"));
  ASSERT_EQ(jflect::read<T4>("[\"hello\",\"beautiful\",\"world\"]"), T4("hello", "beautiful", "world"));
}